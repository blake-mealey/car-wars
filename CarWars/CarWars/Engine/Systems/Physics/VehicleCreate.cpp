// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2017 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#include <new>
#include "VehicleCreate.h"
#include "PxPhysicsAPI.h"

using namespace physx;

// TODO: Replace with Mesh and/or move to VehicleComponent

static PxConvexMesh* createConvexMesh(const PxVec3* verts, const PxU32 numVerts, PxPhysics& physics, PxCooking& cooking)
{
	// Create descriptor for convex mesh
	PxConvexMeshDesc convexDesc;
	convexDesc.points.count			= numVerts;
	convexDesc.points.stride		= sizeof(PxVec3);
	convexDesc.points.data			= verts;
	convexDesc.flags				= PxConvexFlag::eCOMPUTE_CONVEX;

	PxConvexMesh* convexMesh = NULL;
	PxDefaultMemoryOutputStream buf;
	if(cooking.cookConvexMesh(convexDesc, buf))
	{
		PxDefaultMemoryInputData id(buf.getData(), buf.getSize());
		convexMesh = physics.createConvexMesh(id);
	}

	return convexMesh;
}

PxConvexMesh* createWheelMesh(const PxF32 width, const PxF32 radius, PxPhysics& physics, PxCooking& cooking)
{
	PxVec3 points[2*16];
	for(PxU32 i = 0; i < 16; i++)
	{
		const PxF32 cosTheta = PxCos(i*PxPi*2.0f/16.0f);
		const PxF32 sinTheta = PxSin(i*PxPi*2.0f/16.0f);
		const PxF32 y = radius*cosTheta;
		const PxF32 z = radius*sinTheta;
		points[2*i+0] = PxVec3(-width/2.0f, y, z);
		points[2*i+1] = PxVec3(+width/2.0f, y, z);
	}

	return createConvexMesh(points,32,physics,cooking);
}

void customizeVehicleToLengthScale(const PxReal lengthScale, PxRigidDynamic* rigidDynamic, PxVehicleWheelsSimData* wheelsSimData, PxVehicleDriveSimData* driveSimData)
{
	//Rigid body center of mass and moment of inertia.
	{
		PxTransform t = rigidDynamic->getCMassLocalPose();
		t.p *= lengthScale;
		rigidDynamic->setCMassLocalPose(t);

		PxVec3 moi = rigidDynamic->getMassSpaceInertiaTensor();
		moi *= (lengthScale*lengthScale);
		rigidDynamic->setMassSpaceInertiaTensor(moi);
	}

	//Wheels, suspensions, wheel centers, tire/susp force application points.
	{
		for(PxU32 i = 0; i < wheelsSimData->getNbWheels(); i++)
		{
			PxVehicleWheelData wheelData = wheelsSimData->getWheelData(i);
			wheelData.mRadius *= lengthScale;
			wheelData.mWidth *= lengthScale;
			wheelData.mDampingRate *= lengthScale*lengthScale;
			wheelData.mMaxBrakeTorque *= lengthScale*lengthScale;
			wheelData.mMaxHandBrakeTorque *= lengthScale*lengthScale;
			wheelData.mMOI *= lengthScale*lengthScale;
			wheelsSimData->setWheelData(i, wheelData);

			PxVehicleSuspensionData suspData = wheelsSimData->getSuspensionData(i);
			suspData.mMaxCompression *= lengthScale;
			suspData.mMaxDroop *= lengthScale;
			wheelsSimData->setSuspensionData(i, suspData);

			PxVec3 v = wheelsSimData->getWheelCentreOffset(i);
			v *= lengthScale;
			wheelsSimData->setWheelCentreOffset(i,v);

			v = wheelsSimData->getSuspForceAppPointOffset(i);
			v *= lengthScale;
			wheelsSimData->setSuspForceAppPointOffset(i,v);

			v = wheelsSimData->getTireForceAppPointOffset(i);
			v *= lengthScale;
			wheelsSimData->setTireForceAppPointOffset(i,v);
		}
	}

	//Slow forward speed correction.
	{
		wheelsSimData->setSubStepCount(5.0f*lengthScale, 3, 1);
		wheelsSimData->setMinLongSlipDenominator(4.0f*lengthScale);
	}

	//Engine
	if(driveSimData)
	{
		PxVehicleEngineData engineData = driveSimData->getEngineData();
		engineData.mMOI *= lengthScale*lengthScale;
		engineData.mPeakTorque *= lengthScale*lengthScale;
		engineData.mDampingRateFullThrottle *= lengthScale*lengthScale;
		engineData.mDampingRateZeroThrottleClutchEngaged *= lengthScale*lengthScale;
		engineData.mDampingRateZeroThrottleClutchDisengaged *= lengthScale*lengthScale;
		driveSimData->setEngineData(engineData);
	}

	//Clutch.
	if(driveSimData)
	{
		PxVehicleClutchData clutchData = driveSimData->getClutchData();
		clutchData.mStrength *= lengthScale*lengthScale;
		driveSimData->setClutchData(clutchData);
	}	

	//Scale the collision meshes too.
	{
		PxShape* shapes[16];
		const PxU32 nbShapes = rigidDynamic->getShapes(shapes, 16);
		for(PxU32 i = 0; i < nbShapes; i++)
		{
			switch(shapes[i]->getGeometryType())
			{
			case PxGeometryType::eSPHERE:
				{
					PxSphereGeometry sphere;
					shapes[i]->getSphereGeometry(sphere);
					sphere.radius *= lengthScale;
					shapes[i]->setGeometry(sphere);
				}
				break;
			case PxGeometryType::ePLANE:
				PX_ASSERT(false);
				break;
			case PxGeometryType::eCAPSULE:
				{
					PxCapsuleGeometry capsule;
					shapes[i]->getCapsuleGeometry(capsule);
					capsule.radius *= lengthScale;
					capsule.halfHeight*= lengthScale;
					shapes[i]->setGeometry(capsule);
				}
				break;
			case PxGeometryType::eBOX:
				{
					PxBoxGeometry box;
					shapes[i]->getBoxGeometry(box);
					box.halfExtents *= lengthScale;
					shapes[i]->setGeometry(box);
				}
				break;
			case PxGeometryType::eCONVEXMESH:
				{
					PxConvexMeshGeometry convexMesh;
					shapes[i]->getConvexMeshGeometry(convexMesh);
					convexMesh.scale.scale *= lengthScale;
					shapes[i]->setGeometry(convexMesh);
				}
				break;
			case PxGeometryType::eTRIANGLEMESH:
				{
					PxTriangleMeshGeometry triMesh;
					shapes[i]->getTriangleMeshGeometry(triMesh);
					triMesh.scale.scale *= lengthScale;
					shapes[i]->setGeometry(triMesh);
				}
				break;
			case PxGeometryType::eHEIGHTFIELD:
				{
					PxHeightFieldGeometry hf;
					shapes[i]->getHeightFieldGeometry(hf);
					hf.columnScale *= lengthScale;
					hf.heightScale *= lengthScale;
					hf.rowScale *= lengthScale;
					shapes[i]->setGeometry(hf);
				}
				break;
			case PxGeometryType::eINVALID:
			case PxGeometryType::eGEOMETRY_COUNT:
				break;
			}
		}
	}
}

void customizeVehicleToLengthScale(const PxReal lengthScale, PxRigidDynamic* rigidDynamic, PxVehicleWheelsSimData* wheelsSimData, PxVehicleDriveSimData4W* driveSimData)
{
	customizeVehicleToLengthScale(lengthScale, rigidDynamic, wheelsSimData, static_cast<PxVehicleDriveSimData*>(driveSimData));

	//Ackermann geometry.
	if(driveSimData)
	{
		PxVehicleAckermannGeometryData ackermannData = driveSimData->getAckermannGeometryData();
		ackermannData.mAxleSeparation *= lengthScale;
		ackermannData.mFrontWidth *= lengthScale;
		ackermannData.mRearWidth *= lengthScale;
		driveSimData->setAckermannGeometryData(ackermannData);
	}
}
