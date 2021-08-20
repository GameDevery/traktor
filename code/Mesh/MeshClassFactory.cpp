#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Class/IRuntimeDelegate.h"
#include "Core/Class/Boxes/BoxedAlignedVector.h"
#include "Core/Class/Boxes/BoxedIntervalTransform.h"
#include "Mesh/IMeshParameterCallback.h"
#include "Mesh/MeshClassFactory.h"
#include "Mesh/MeshComponent.h"
#include "Mesh/Blend/BlendMesh.h"
#include "Mesh/Blend/BlendMeshComponent.h"
#include "Mesh/Composite/CompositeMeshComponent.h"
#include "Mesh/Indoor/IndoorMesh.h"
#include "Mesh/Indoor/IndoorMeshComponent.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Mesh/Instance/InstanceMeshComponent.h"
#include "Mesh/Lod/AutoLodMesh.h"
#include "Mesh/Lod/AutoLodMeshComponent.h"
#include "Mesh/Partition/PartitionMesh.h"
#include "Mesh/Partition/PartitionMeshComponent.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Mesh/Skinned/SkinnedMeshComponent.h"
#include "Mesh/Static/StaticMesh.h"
#include "Mesh/Static/StaticMeshComponent.h"
#include "Mesh/Stream/StreamMesh.h"
#include "Mesh/Stream/StreamMeshComponent.h"
#include "Render/Buffer.h"
#include "Render/ITexture.h"
#include "Render/RenderClassFactory.h"
#include "Render/Context/ProgramParameters.h"
#include "Resource/ResourceCast.h"

namespace traktor
{
	namespace mesh
	{
		namespace
		{

class DelegateMeshParameterCallback : public RefCountImpl< IMeshParameterCallback >
{
public:
	DelegateMeshParameterCallback(IRuntimeDelegate* callback)
	:	m_callback(callback)
	{
	}

	virtual void setCombination(render::Shader* shader) const
	{
	}

	virtual void setParameters(render::ProgramParameters* programParameters) const
	{
		m_programParameters.setProgramParameters(programParameters);
		Any argv[] =
		{
			CastAny< render::BoxedProgramParameters* >::set(&m_programParameters)
		};
		m_callback->call(sizeof_array(argv), argv);
		m_programParameters.setProgramParameters(nullptr);
	}

private:
	Ref< IRuntimeDelegate > m_callback;
	mutable render::BoxedProgramParameters m_programParameters;
};

void MeshComponent_setParameterCallback(MeshComponent* self, IRuntimeDelegate* callback)
{
	if (callback)
		self->setParameterCallback(new DelegateMeshParameterCallback(callback));
	else
		self->setParameterCallback(nullptr);
}

const IntervalTransform& MeshComponent_getTransform(MeshComponent* self)
{
	return self->getTransform();
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.MeshClassFactory", 0, MeshClassFactory, IRuntimeClassFactory)

void MeshClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classIMesh = new AutoRuntimeClass< IMesh >();
	registrar->registerClass(classIMesh);

	auto classMeshComponent = new AutoRuntimeClass< MeshComponent >();
	classMeshComponent->addMethod("setParameterCallback", &MeshComponent_setParameterCallback);
	classMeshComponent->addProperty("transform", &MeshComponent_getTransform);
	registrar->registerClass(classMeshComponent);

	auto classBlendMesh = new AutoRuntimeClass< BlendMesh >();
	registrar->registerClass(classBlendMesh);

	auto classBlendMeshComponent = new AutoRuntimeClass< BlendMeshComponent >();
	classBlendMeshComponent->addMethod("setBlendWeights", &BlendMeshComponent::setBlendWeights);
	classBlendMeshComponent->addMethod("getBlendWeights", &BlendMeshComponent::getBlendWeights);
	registrar->registerClass(classBlendMeshComponent);

	auto classCompositeMeshComponent = new AutoRuntimeClass< CompositeMeshComponent >();
	classCompositeMeshComponent->addConstructor();
	classCompositeMeshComponent->addMethod("removeAll", &CompositeMeshComponent::removeAll);
	classCompositeMeshComponent->addMethod("remove", &CompositeMeshComponent::remove);
	classCompositeMeshComponent->addMethod("add", &CompositeMeshComponent::add);
	registrar->registerClass(classCompositeMeshComponent);

	auto classIndoorMesh = new AutoRuntimeClass< IndoorMesh >();
	registrar->registerClass(classIndoorMesh);

	auto classIndoorMeshComponent = new AutoRuntimeClass< IndoorMeshComponent >();
	registrar->registerClass(classIndoorMeshComponent);

	auto classInstanceMesh = new AutoRuntimeClass< InstanceMesh >();
	registrar->registerClass(classInstanceMesh);

	auto classInstanceMeshComponent = new AutoRuntimeClass< InstanceMeshComponent >();
	registrar->registerClass(classInstanceMeshComponent);

	auto classAutoLodMesh = new AutoRuntimeClass< AutoLodMesh >();
	registrar->registerClass(classAutoLodMesh);

	auto classAutoLodMeshComponent = new AutoRuntimeClass< AutoLodMeshComponent >();
	registrar->registerClass(classAutoLodMeshComponent);

	auto classPartitionMesh = new AutoRuntimeClass< PartitionMesh >();
	registrar->registerClass(classPartitionMesh);

	auto classPartitionMeshComponent = new AutoRuntimeClass< PartitionMeshComponent >();
	registrar->registerClass(classPartitionMeshComponent);

	auto classSkinnedMesh = new AutoRuntimeClass< SkinnedMesh >();
	registrar->registerClass(classSkinnedMesh);

	auto classSkinnedMeshComponent = new AutoRuntimeClass< SkinnedMeshComponent >();
	registrar->registerClass(classSkinnedMeshComponent);

	auto classStaticMesh = new AutoRuntimeClass< StaticMesh >();
	registrar->registerClass(classStaticMesh);

	auto classStaticMeshComponent = new AutoRuntimeClass< StaticMeshComponent >();
	registrar->registerClass(classStaticMeshComponent);

	auto classStreamMesh = new AutoRuntimeClass< StreamMesh >();
	registrar->registerClass(classStreamMesh);

	auto classStreamMeshComponent = new AutoRuntimeClass< StreamMeshComponent >();
	classStreamMeshComponent->addProperty("frameCount", &StreamMeshComponent::getFrameCount);
	classStreamMeshComponent->addProperty("frame", &StreamMeshComponent::setFrame, &StreamMeshComponent::getFrame);
	registrar->registerClass(classStreamMeshComponent);
}

	}
}
