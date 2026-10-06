// Fill out your copyright notice in the Description page of Project Settings.

#include "ABullet.h"
#include "Kismet/GameplayStatics.h"
#include "UGameEventSystem.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"

AABullet::AABullet()  
{
	bReplicates = true;
	SetReplicateMovement(true);
	
	PrimaryActorTick.bCanEverTick = false;
	
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(Collision);

	Collision->InitSphereRadius(8.0f);
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionResponseToAllChannels(ECR_Overlap);
	Collision->SetNotifyRigidBodyCollision(true);
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Collision);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));

	ProjectileMovement->InitialSpeed = 3000.0f;
	ProjectileMovement->MaxSpeed = 3000.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	
	InitialLifeSpan = 1.0f;
}

// Called when the game starts or when spawned
void AABullet::BeginPlay()
{
	Super::BeginPlay();
	PreviousLocation = GetActorLocation();
	
	Collision->OnComponentHit.AddDynamic(this, &AABullet::OnBulletHit);
	if (GetOwner())
	{
		Collision->IgnoreActorWhenMoving(GetOwner(), true);
	}
	
	UUGameEventSystem* GameEventSystem = GetWorld()->GetSubsystem<UUGameEventSystem>();
	if (GameEventSystem != nullptr && HasAuthority() == true)
	{
		GameEventSystem->ServerBullets.Add(this);
	}	
}

void AABullet::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetNetMode() != NM_DedicatedServer)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, GetActorLocation());
	}	
	Super::EndPlay(EndPlayReason);
}

void AABullet::OnBulletHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor == nullptr || OtherActor == this || OtherActor == GetOwner())
		return;
	
	if (HasAuthority() == true)
	{
		UUGameEventSystem* GameEventSystem = GetWorld()->GetSubsystem<UUGameEventSystem>();
		if (GameEventSystem != nullptr)
		{
			FGameEvent Event;
			Event.Type = EGameEventType::GunShoot;
			Event.GunShootEvent.EffectRadius = 300.f;
			Event.GunShootEvent.Location = Hit.ImpactPoint;
			
			GameEventSystem->AddGameEvent(Event);
			
			UE_LOG(LogTemp, Warning, TEXT("GunShoot: OtherActor=%s, OtherActorLocation=%s, HitLocation=%s"), 
				*GetNameSafe(OtherActor), *OtherActor->GetActorLocation().ToString(), *Event.GunShootEvent.Location.ToString());
		}
		
		Destroy();
	}
}
