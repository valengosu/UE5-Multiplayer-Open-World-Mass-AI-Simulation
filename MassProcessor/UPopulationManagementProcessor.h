#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassZoneGraphNavigationFragments.h"
#include "PopulationSubsystem.h"
#include "UPopulationManagementProcessor.generated.h"

UCLASS()
class OPENWORLD_API UPopulationManagementProcessor : public UMassProcessor
{
	GENERATED_BODY()
	
public:
	UPopulationManagementProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	virtual bool ShouldAllowQueryBasedPruning(const bool bRuntimeMode = true) const override;
	int IsInsideLanes(const FMassZoneGraphLaneLocationFragment& Location, const TArray<FZoneGraphLaneSection>& Sections);
	
private:
	FMassEntityQuery EntityQuery;
};