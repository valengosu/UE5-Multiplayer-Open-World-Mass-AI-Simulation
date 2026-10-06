#include "UReplicationAgentCleanup.h"
#include "MassCommonFragments.h"
#include "MassCrowdBubble.h"
#include "MassReplicationFragments.h"
#include "MassExecutionContext.h"
#include "MassReplicationSubsystem.h"

class FMassCrowdClientBubbleHandler;

UReplicationAgentCleanup::UReplicationAgentCleanup()
{
	ObservedType = FMassNetworkIDFragment::StaticStruct();
	ObservedOperations = EMassObservedOperationFlags::Remove;

	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Server);
}

void UReplicationAgentCleanup::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.Initialize(EntityManager);

	EntityQuery.AddRequirement<FMassNetworkIDFragment>(EMassFragmentAccess::ReadOnly);
	
	EntityQuery.AddSharedRequirement<FMassReplicationSharedFragment>(EMassFragmentAccess::ReadWrite);

	//EntityQuery.AddSubsystemRequirement<UMassReplicationSubsystem>(EMassFragmentAccess::ReadWrite);

	EntityQuery.RegisterWithProcessor(*this);
}

// UE5.7 MassReplication workaround:
// Explicitly remove destroyed replicated Mass agents from each client bubble
// and cached replication data so FastArray removal reaches remote clients.
void UReplicationAgentCleanup::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UMassReplicationSubsystem* ReplicationSubsystem = GetWorld()->GetSubsystem<UMassReplicationSubsystem>();
	check(ReplicationSubsystem);
	
	EntityQuery.ForEachEntityChunk(
			Context,
			[ReplicationSubsystem](FMassExecutionContext& ChunkContext)
			{
				const TConstArrayView<FMassNetworkIDFragment> NetworkIDs = ChunkContext.GetFragmentView<FMassNetworkIDFragment>();
				FMassReplicationSharedFragment& ReplicationSharedFragment = ChunkContext.GetMutableSharedFragment<FMassReplicationSharedFragment>();
				
				for (FMassExecutionContext::FEntityIterator EntityIt = ChunkContext.CreateEntityIterator(); EntityIt; ++EntityIt)
				{
					const FMassEntityHandle Entity = ChunkContext.GetEntity(EntityIt);

					const FMassNetworkID NetID = NetworkIDs[EntityIt].NetID;

					for (const FMassClientHandle ClientHandle : ReplicationSubsystem->GetClientReplicationHandles())
					{
						AMassCrowdClientBubbleInfo& CrowdBubbleInfo = ReplicationSharedFragment.GetTypedClientBubbleInfoChecked<AMassCrowdClientBubbleInfo>(ClientHandle);
						FMassCrowdClientBubbleHandler& Bubble = CrowdBubbleInfo.GetCrowdSerializer().Bubble; 
						Bubble.RemoveAgent(NetID);
						
						ReplicationSubsystem->GetMutableClientReplicationInfoChecked(ClientHandle).AgentsData.Remove(Entity);
					}

					UE_LOG(LogTemp, Warning, TEXT("Replication cleanup executed"));
				}
			});
}