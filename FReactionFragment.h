#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "../UGameEventSystem.h"
#include "FReactionFragment.generated.h"

USTRUCT()
struct FReactionFragment : public FMassFragment
{
	GENERATED_BODY()
	
	EGameEventType EventType = EGameEventType::None;
	FVector EventLocation = FVector::ZeroVector;
	float EffectRadius = 0.0f;
	FMassEntityHandle SourceCharacterHandle;
	bool bPending = false;
};