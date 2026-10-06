#include "MyMassCrowdRepresentationSubsystem.h"
#include "MassActorSubsystem.h"
#include "MassEntityView.h"
#include "MassRepresentationFragments.h"

void UMyMassCrowdRepresentationSubsystem::ReleaseMassRepresentationForRealActorTakeover(
	FMassEntityManager& InEntityManager, const FMassEntityHandle MassAgent, bool bImmediate)
{
	FMassEntityView EntityView(InEntityManager, MassAgent);
	FMassActorFragment& ActorInfo = EntityView.GetFragmentData<FMassActorFragment>();
	
	AActor* Actor = ActorInfo.GetOwnedByMassMutable();
	if (Actor)
	{
		// WARNING!
		// Need to reset before ReleaseTemplateActorOrCancelSpawning as this action might move the entity to a new archetype and
		// so the Fragment passed in parameters would not be valid anymore.
		ActorInfo.ResetAndUpdateHandleMap(nullptr);
	}
	
	FMassRepresentationFragment* Representation = &EntityView.GetFragmentData<FMassRepresentationFragment>();
	
	// Try releasing both as we can have a low res actor and a high res spawning request
	if (Representation->HighResTemplateActorIndex != INDEX_NONE)
	{
		ReleaseTemplateActorOrCancelSpawning(MassAgent, Representation->HighResTemplateActorIndex, Actor, Representation->ActorSpawnRequestHandle, bImmediate);
	}
	
	FMassEntityView EntityViewAfterHigh(InEntityManager, MassAgent);
	Representation = &EntityViewAfterHigh.GetFragmentData<FMassRepresentationFragment>();
	
	if (Representation->LowResTemplateActorIndex != Representation->HighResTemplateActorIndex && Representation->LowResTemplateActorIndex != INDEX_NONE)
	{
		ReleaseTemplateActorOrCancelSpawning(MassAgent, Representation->LowResTemplateActorIndex, Actor, Representation->ActorSpawnRequestHandle, bImmediate);
	}
	
	FMassEntityView EntityViewAfterLow(InEntityManager, MassAgent);
	Representation = &EntityViewAfterLow.GetFragmentData<FMassRepresentationFragment>();
	check(!Representation->ActorSpawnRequestHandle.IsValid());
}

bool UMyMassCrowdRepresentationSubsystem::ReleaseTemplateActorOrCancelSpawning(const FMassEntityHandle MassAgent,
	const int16 TemplateActorIndex, AActor* ActorToRelease, FMassActorSpawnRequestHandle& SpawnRequestHandle,
	bool bImmediate)
{
	// First try to cancel the spawning request, then try to release the actor
	if (CancelSpawningInternalTakeOver(TemplateActorIndex, SpawnRequestHandle, bImmediate) 
		|| ReleaseTemplateActorInternal(TemplateActorIndex, ActorToRelease, bImmediate))
	{ 
		int32* RefCountPtr = HandledMassAgents.Find(MassAgent);
		if (RefCountPtr == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("[RealActorTakeOver] Missing HandledMassAgents entry. Entity=(%d,%d) TemplateIndex=%d Actor=%s SpawnHandleValid=%d"), MassAgent.Index, MassAgent.SerialNumber, TemplateActorIndex, *GetNameSafe(ActorToRelease), SpawnRequestHandle.IsValid());
			return true;
		}
		
		const int32 RefCount = --(HandledMassAgents.FindChecked(MassAgent));
		checkf(RefCount >= 0, TEXT("RefCount are expected to be greater than or equal to 0"));
		if (RefCount == 0)
		{
			HandledMassAgents.Remove(MassAgent);
		}
		return true;
	}

	return false;
}


bool UMyMassCrowdRepresentationSubsystem::CancelSpawningInternalTakeOver(int16 TemplateActorIndex, FMassActorSpawnRequestHandle& SpawnRequestHandle, const bool bImmediateActorRelease)
{
	UE_MT_SCOPED_READ_ACCESS(TemplateActorsMTAccessDetector);
	if (!TemplateActors.IsValidIndex(TemplateActorIndex))
	{
		UE_LOG(LogTemp, Error, TEXT("Template actor type %i is not referring a valid type"), TemplateActorIndex);
		return false;
	}
	const TSubclassOf<AActor> TemplateToRelease = TemplateActors[TemplateActorIndex].Actor;
	
	// Check if there is something to cancel
	if (!SpawnRequestHandle.IsValid())
	{
		return false;
	}

	check(ActorSpawnerSubsystem);
	const FMassActorSpawnRequest& SpawnRequest = ActorSpawnerSubsystem->GetMutableSpawnRequest<FMassActorSpawnRequest>(SpawnRequestHandle);
	// Check if the spawning request matches the template actor
	if (SpawnRequest.Template != TemplateToRelease)
	{
		UE_LOG(LogTemp, Error, TEXT("[RealActorTakeOver] TemplateMismatch Request=%s Release=%s TemplateIndex=%d HandleIndex=%d"), *GetNameSafe(SpawnRequest.Template), *GetNameSafe(TemplateToRelease), TemplateActorIndex, SpawnRequestHandle.GetIndex());
		
		TemplateActorIndex = INDEX_NONE;
		for (int32 Index = 0; Index < TemplateActors.Num(); ++Index)
		{
			if (TemplateActors[Index].Actor == SpawnRequest.Template)
			{
				TemplateActorIndex = static_cast<int16>(Index);
				break;
			}
		}
		
		if (TemplateActorIndex == INDEX_NONE)
		{
			UE_LOG(LogTemp, Error, TEXT("[RealActorTakeOver] Cannot find TemplateIndex for Request=%s"), *GetNameSafe(SpawnRequest.Template));
			return false;
		}
	}

	if (SpawnRequest.SpawnStatus == ESpawnRequestStatus::Succeeded)
	{
		check(SpawnRequest.SpawnedActor);
		ReleaseTemplateActorInternal(TemplateActorIndex, SpawnRequest.SpawnedActor, bImmediateActorRelease);
	}

	// Remove the spawn request
	ensureMsgf(ActorSpawnerSubsystem->RemoveActorSpawnRequest(SpawnRequestHandle), TEXT("Unable to remove a valid spawn request"));

	return true;
}
