#pragma once

#include "MassCrowdReplicator.h"
#include "UMyCrowdReplicator.generated.h"

UCLASS()
class UMyCrowdReplicator : public UMassCrowdReplicator
{
	GENERATED_BODY()

protected:
	virtual void ProcessClientReplication(FMassExecutionContext& Context, FMassReplicationContext& ReplicationContext) override;
};