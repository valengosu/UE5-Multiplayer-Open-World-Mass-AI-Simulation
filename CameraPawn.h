#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CameraPawn.generated.h"

class UCameraComponent;
class UFloatingPawnMovement;
class UInputAction;
struct FInputActionValue;

UCLASS()
class OPENWORLD_API ACameraPawn : public APawn
{
	GENERATED_BODY()

public:
	ACameraPawn();

protected:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// WASD
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveAction;

	// Mouse movement
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MouseLookAction;

	// Right mouse button
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* RightMouseAction;
	
	// Input callbacks
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	void RightMousePressed();
	void RightMouseReleased();
	
	UFUNCTION(Server, Unreliable)
	void ServerMove(FVector2D MovementVector);
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UFloatingPawnMovement* FloatingPawnMovement;

	bool bRightMousePressed = false;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float NormalSpeed = 1500.0f;
};