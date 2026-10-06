#include "UMyCrowdReplicator.h"
#include "MassCrowdBubble.h"
#include "MassExecutionContext.h"
#include "UMyCrowdBubble.h"

void UMyCrowdReplicator::ProcessClientReplication(FMassExecutionContext& Context, FMassReplicationContext& ReplicationContext)
{
#if UE_REPLICATION_COMPILE_SERVER_CODE

	FMassReplicationProcessorPathHandler PathHandler;
	FMassReplicationProcessorPositionYawHandler PositionYawHandler;
	FMassReplicationSharedFragment* RepSharedFrag = nullptr;

	auto CacheViewsCallback = [&RepSharedFrag, &PathHandler, &PositionYawHandler](FMassExecutionContext& Context)
	{
		PathHandler.CacheFragmentViews(Context);
		PositionYawHandler.CacheFragmentViews(Context);
		RepSharedFrag = &Context.GetMutableSharedFragment<FMassReplicationSharedFragment>();
		check(RepSharedFrag);
	};

	auto AddEntityCallback = [&RepSharedFrag, &PathHandler, &PositionYawHandler](FMassExecutionContext& Context, const int32 EntityIdx, FReplicatedCrowdAgent& InReplicatedAgent, const FMassClientHandle ClientHandle)->FMassReplicatedAgentHandle
	{
		AMyCrowdClientBubbleInfo& CrowdBubbleInfo = RepSharedFrag->GetTypedClientBubbleInfoChecked<AMyCrowdClientBubbleInfo>(ClientHandle);

		//PathHandler.AddEntity(EntityIdx, InReplicatedAgent.GetReplicatedPathDataMutable());
		PositionYawHandler.AddEntity(EntityIdx, InReplicatedAgent.GetReplicatedPositionYawDataMutable());

		return CrowdBubbleInfo.GetCrowdSerializer().Bubble.AddAgent(Context.GetEntity(EntityIdx), InReplicatedAgent);
	};

	auto ModifyEntityCallback = [&RepSharedFrag, &PathHandler, &PositionYawHandler](FMassExecutionContext& Context, const int32 EntityIdx, const EMassLOD::Type LOD, const double Time, const FMassReplicatedAgentHandle Handle, const FMassClientHandle ClientHandle)
	{
		AMyCrowdClientBubbleInfo& CrowdBubbleInfo = RepSharedFrag->GetTypedClientBubbleInfoChecked<AMyCrowdClientBubbleInfo>(ClientHandle);
		FMyCrowdClientBubbleHandler& Bubble = CrowdBubbleInfo.GetCrowdSerializer().Bubble;

		//const bool bLastClient = RepSharedFrag->CachedClientHandles.Last() == ClientHandle;
		//PathHandler.ModifyEntity<FCrowdFastArrayItem>(Handle, EntityIdx, Bubble.GetPathHandlerMutable(), bLastClient);

		const FVector CurrentPosition = Context.GetFragmentView<FTransformFragment>()[EntityIdx].GetTransform().GetLocation();
		const float CurrentYaw = static_cast<float>(FMath::DegreesToRadians(Context.GetFragmentView<FTransformFragment>()[EntityIdx].GetTransform().Rotator().Yaw));
		
		const FReplicatedCrowdAgent* ReplicatedAgent = Bubble.GetAgent(Handle);
		if (ReplicatedAgent == nullptr)
			return;
		
		const FVector LastReplicatedPosition = ReplicatedAgent->GetReplicatedPositionYawData().GetPosition();
		const float LastReplicatedYaw = ReplicatedAgent->GetReplicatedPositionYawData().GetYaw();
		const float YawDelta = FMath::Abs(FMath::FindDeltaAngleRadians(CurrentYaw, LastReplicatedYaw));
		
		const float Offset = 3.f;
		const float YawOffset = FMath::DegreesToRadians(5.0f);
		
		//if (FVector::DistSquared(CurrentPosition, LastReplicatedPosition) < FMath::Square(Offset) && YawDelta < YawOffset)
		//	return;
		
		PositionYawHandler.ModifyEntity<FCrowdFastArrayItem>(Handle, EntityIdx, Bubble.GetTransformHandlerMutable());
	};

	auto RemoveEntityCallback = [&RepSharedFrag](FMassExecutionContext& Context, const FMassReplicatedAgentHandle Handle, const FMassClientHandle ClientHandle)
	{
		AMyCrowdClientBubbleInfo& CrowdBubbleInfo = RepSharedFrag->GetTypedClientBubbleInfoChecked<AMyCrowdClientBubbleInfo>(ClientHandle);

		CrowdBubbleInfo.GetCrowdSerializer().Bubble.RemoveAgentChecked(Handle);
	};

	CalculateClientReplication<FCrowdFastArrayItem>(Context, ReplicationContext, CacheViewsCallback, AddEntityCallback, ModifyEntityCallback, RemoveEntityCallback);
#endif // UE_REPLICATION_COMPILE_SERVER_CODE
}
