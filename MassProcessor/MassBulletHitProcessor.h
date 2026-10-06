#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassBulletHitProcessor.generated.h"

UCLASS()
class OPENWORLD_API UMassBulletHitProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UMassBulletHitProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};