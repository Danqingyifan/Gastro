#include "Character/FalloutCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/GastroAbilitySystemComponent.h"
#include "Input/FalloutFPSInputConfig.h"
#include "Input/FalloutInputTags.h"
#include "Player/FalloutFPSPlayerController.h"

AFalloutCharacter::AFalloutCharacter()
{
    // Keep ACharacter's engine tick (jump bookkeeping and based movement).
    PrimaryActorTick.bCanEverTick = true;
    bUseControllerRotationYaw = true;
    GetCapsuleComponent()->InitCapsuleSize(34.f, 96.f);
    auto* Movement = GetCharacterMovement();
    Movement->bOrientRotationToMovement = false;
    Movement->MaxWalkSpeed = NormalWalkSpeed;
    Movement->MaxWalkSpeedCrouched = CrouchedWalkSpeed;
    Movement->MaxAcceleration = 12000.f;
    Movement->BrakingDecelerationWalking = 12000.f;
    Movement->GroundFriction = 8.f;
    Movement->BrakingFrictionFactor = 1.f;
    Movement->JumpZVelocity = 700.f;
    Movement->GravityScale = 2.5f;
    Movement->AirControl = 0.8f;
    Movement->AirControlBoostMultiplier = 1.f;
    Movement->BrakingDecelerationFalling = 0.f;
    Movement->SetCrouchedHalfHeight(56.f);
    Movement->GetNavAgentPropertiesRef().bCanCrouch = true;
    JumpMaxCount = 1;
    JumpMaxHoldTime = 0.f;

    FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
    FirstPersonCamera->SetRelativeLocation(FVector(0, 0, 72));
    FirstPersonCamera->bUsePawnControlRotation = true;
    FirstPersonCamera->FieldOfView = 100.f;
    FirstPersonCamera->bEnableFirstPersonFieldOfView = true;
    FirstPersonCamera->bEnableFirstPersonScale = true;
    FirstPersonCamera->FirstPersonFieldOfView = 70.f;
    FirstPersonCamera->FirstPersonScale = 0.6f;

    // The arms follow the camera directly; this lesson adds no weapon motion.
    FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
    FirstPersonMesh->SetupAttachment(FirstPersonCamera);
    FirstPersonMesh->SetRelativeLocationAndRotation(FVector(0, 0, -168), FRotator(0, -90, 0));
    FirstPersonMesh->SetOnlyOwnerSee(true);
    FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
    FirstPersonMesh->SetCollisionProfileName(TEXT("NoCollision"));
    FirstPersonMesh->SetCastShadow(false);

    FirstPersonWeapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonWeapon"));
    FirstPersonWeapon->SetupAttachment(FirstPersonMesh, WeaponHandSocket);
    FirstPersonWeapon->SetRelativeTransform(WeaponGripOffset);
    FirstPersonWeapon->SetOnlyOwnerSee(true);
    FirstPersonWeapon->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
    FirstPersonWeapon->SetCollisionProfileName(TEXT("NoCollision"));
    FirstPersonWeapon->SetCastShadow(false);

    GetMesh()->SetRelativeLocationAndRotation(FVector(0, 0, -96), FRotator(0, -90, 0));
    GetMesh()->SetOwnerNoSee(true);
    GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;
    GetMesh()->SetCollisionProfileName(TEXT("NoCollision"));
    GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
}

void AFalloutCharacter::BeginPlay()
{
    Super::BeginPlay();
    GetCharacterMovement()->MaxWalkSpeed = NormalWalkSpeed;
    GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchedWalkSpeed;
    VisualEyeHeight = bIsCrouched ? CrouchingEyeHeight : StandingEyeHeight;
    // A mannequin is a placeholder viewmodel; never render its head or legs in the 1P view.
    FirstPersonMesh->HideBoneByName(TEXT("neck_01"), EPhysBodyOp::PBO_None);
    FirstPersonMesh->HideBoneByName(TEXT("thigh_l"), EPhysBodyOp::PBO_None);
    FirstPersonMesh->HideBoneByName(TEXT("thigh_r"), EPhysBodyOp::PBO_None);
    FirstPersonWeapon->AttachToComponent(FirstPersonMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponHandSocket);
    FirstPersonWeapon->SetRelativeTransform(WeaponGripOffset);
    PlaceCamera();
}

void AFalloutCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    if (auto* Old = Cast<UEnhancedInputComponent>(AxisInputComponent.Get()))
        for (uint32 Handle : AxisBindingHandles) Old->RemoveBindingByHandle(Handle);
    AxisBindingHandles.Reset();
    AxisInputComponent.Reset();
    auto* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    auto* PC = Cast<AFalloutFPSPlayerController>(Controller);
    if (!Input || !PC || !FPSInputConfig || !FPSInputConfig->MoveAction || !FPSInputConfig->MouseLookAction)
    {
        UE_LOG(LogTemp, Error, TEXT("FPS character requires its FPS controller, EnhancedInputComponent and valid FPSInputConfig."));
        return;
    }
    AxisInputComponent = Input;
    AxisBindingHandles.Add(Input->BindAction(FPSInputConfig->MoveAction, ETriggerEvent::Triggered, this, &ThisClass::MoveInput).GetHandle());
    AxisBindingHandles.Add(Input->BindAction(FPSInputConfig->MoveAction, ETriggerEvent::Completed, this, &ThisClass::MoveInputReleased).GetHandle());
    AxisBindingHandles.Add(Input->BindAction(FPSInputConfig->MoveAction, ETriggerEvent::Canceled, this, &ThisClass::MoveInputReleased).GetHandle());
    AxisBindingHandles.Add(Input->BindAction(FPSInputConfig->MouseLookAction, ETriggerEvent::Triggered, this, &ThisClass::LookInput).GetHandle());
    PC->ConfigureFPSInput(Input, FPSInputConfig);
}

void AFalloutCharacter::MoveInput(const FInputActionValue& Value) { Move(Value.Get<FVector2D>()); }
void AFalloutCharacter::MoveInputReleased(const FInputActionValue& Value)
{
    LastMoveAxis = FVector2D::ZeroVector;
    RefreshMovementSpeed();
}
void AFalloutCharacter::LookInput(const FInputActionValue& Value) { Look(Value.Get<FVector2D>()); }

void AFalloutCharacter::Move(const FVector2D& Axis)
{
    if (!Controller || Controller->IsMoveInputIgnored()) return;
    const FVector2D Clamped = Axis.GetClampedToMaxSize(1.f);
    LastMoveAxis = Clamped;
    const FRotator Yaw(0, Controller->GetControlRotation().Yaw, 0);
    AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::X), Clamped.Y);
    AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::Y), Clamped.X);
    RefreshMovementSpeed();
}

void AFalloutCharacter::Look(const FVector2D& MouseDelta)
{
    if (auto* PC = Cast<AFalloutFPSPlayerController>(Controller))
    {
        PC->ApplyMouseLook(MouseDelta);
    }
}

void AFalloutCharacter::PlaceCamera()
{
    const float HalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    const FVector Feet = GetActorLocation() - FVector(0, 0, HalfHeight);
    const float Scale = GetActorScale3D().Z;
    const FVector Desired = Feet + FVector(0, 0, VisualEyeHeight * Scale);
    // Sweep upward from an interior safe point. Only presentation is clamped, never the capsule.
    const FVector Safe = Feet + FVector(0, 0, FMath::Min(CrouchingEyeHeight * Scale, HalfHeight));
    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(FalloutCamera), false, this);
    FVector CameraLocation = Desired;
    if (GetWorld()->SweepSingleByChannel(Hit, Safe, Desired, FQuat::Identity, ECC_Camera,
        FCollisionShape::MakeSphere(4.f), Params))
        CameraLocation = Hit.bStartPenetrating ? Safe : Hit.Location;
    FirstPersonCamera->SetWorldLocation(CameraLocation);
}

void AFalloutCharacter::CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult)
{
    const float Target = bIsCrouched ? CrouchingEyeHeight : StandingEyeHeight;
    const float Alpha = 1.f - FMath::Exp(-4.60517f * FMath::Max(0.f, DeltaTime) / FMath::Max(0.001f, CrouchBlendTime));
    VisualEyeHeight = FMath::Lerp(VisualEyeHeight, Target, Alpha);
    if (FMath::Abs(VisualEyeHeight - Target) < 0.01f) VisualEyeHeight = Target;
    PlaceCamera();
    FirstPersonCamera->GetCameraView(DeltaTime, OutResult);
}

void AFalloutCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
    Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
    bSprintRequested = false;
    if (auto* ASC = Cast<UGastroAbilitySystemComponent>(GetAbilitySystemComponent()))
        ASC->AbilityInputTagReleased(FalloutInputTags::Sprint);
    RefreshMovementSpeed();
    PlaceCamera();
}
void AFalloutCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
    Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
    RefreshMovementSpeed();
    PlaceCamera();
}

void AFalloutCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
    Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
    if (GetCharacterMovement()->MovementMode != MOVE_Walking &&
        GetCharacterMovement()->MovementMode != MOVE_NavWalking && bSprintRequested)
    {
        bSprintRequested = false;
        if (auto* ASC = Cast<UGastroAbilitySystemComponent>(GetAbilitySystemComponent()))
            ASC->AbilityInputTagReleased(FalloutInputTags::Sprint);
    }
    RefreshMovementSpeed();
}

bool AFalloutCharacter::CanStartSprint() const
{
    const auto* Movement = GetCharacterMovement();
    return Movement && Movement->IsMovingOnGround() && !bIsCrouched && !bAiming;
}

bool AFalloutCharacter::IsSprinting() const
{
    const auto* Movement = GetCharacterMovement();
    return bSprintRequested && !bIsCrouched && !bAiming && Movement && Movement->IsMovingOnGround() && LastMoveAxis.Y > 0.1f;
}

void AFalloutCharacter::SetSprintRequested(bool bRequested)
{
    bSprintRequested = bRequested && CanStartSprint();
    RefreshMovementSpeed();
}

void AFalloutCharacter::SetAiming(bool bInAiming)
{
    bAiming = bInAiming;
    if (bAiming)
    {
        bSprintRequested = false;
        if (auto* ASC = Cast<UGastroAbilitySystemComponent>(GetAbilitySystemComponent()))
            ASC->AbilityInputTagReleased(FalloutInputTags::Sprint);
    }
    RefreshMovementSpeed();
}

void AFalloutCharacter::RefreshMovementSpeed()
{
    auto* Movement = GetCharacterMovement();
    if (!Movement) return;
    const bool bSprintSpeed = bSprintRequested && !bIsCrouched && !bAiming &&
        Movement->IsMovingOnGround() && LastMoveAxis.Y > 0.1f;
    Movement->MaxWalkSpeed = bSprintSpeed ? SprintSpeed : NormalWalkSpeed;
}
bool AFalloutCharacter::CanJumpInternal_Implementation() const
{
    return GetCharacterMovement()->IsMovingOnGround() && !bIsCrouched && Super::CanJumpInternal_Implementation();
}
void AFalloutCharacter::ResetActionIntent()
{
    StopJumping();
    UnCrouch();
    bSprintRequested = false;
    bAiming = false;
    LastMoveAxis = FVector2D::ZeroVector;
    RefreshMovementSpeed();
    ConsumeMovementInputVector();
}
void AFalloutCharacter::UnPossessed()
{
    ResetActionIntent();
    Super::UnPossessed();
}
void AFalloutCharacter::EndPlay(const EEndPlayReason::Type Reason)
{
    ResetActionIntent();
    Super::EndPlay(Reason);
}
