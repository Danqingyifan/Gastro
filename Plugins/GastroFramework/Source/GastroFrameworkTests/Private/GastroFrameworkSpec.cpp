#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "GastroTestTypes.h"
#include "GAS/GastroAbilitySystemComponent.h"
#include "GAS/GastroAbilitySet.h"
#include "Components/GastroAbilitySystemBindingComponent.h"
#include "Input/GastroAbilityInputComponent.h"
#include "Input/GastroInputConfig.h"
#include "Characters/GastroCharacterWithOwnedASC.h"
#include "Characters/GastroCharacterWithExternalASC.h"
#include "Characters/GastroPlayerState.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "InputTriggers.h"
#include "GameplayTagsManager.h"
#include "GameFramework/WorldSettings.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/DataValidation.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"

static FGameplayTag TestInput;
static FGameplayTag TestInputChild;
static FGameplayTag TestOther;

BEGIN_DEFINE_SPEC(FGastroFrameworkSpec, "GastroFramework", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
    UWorld* World = nullptr;
    AGastroTestActor* Actor = nullptr;
    UGastroAbilitySystemComponent* ASC = nullptr;
    UGastroAbilitySystemBindingComponent* Binding = nullptr;
    UGastroAbilitySet* Set = nullptr;
    UGastroInputConfig* Config = nullptr;
    UInputAction* Action = nullptr;
    FGastroAbilitySetGrantedHandles Receipt;
    FGameplayAbilitySpecHandle Grant(TSubclassOf<UGastroGameplayAbility> Ability = UGastroTestAbility::StaticClass());
    UGastroTestAbility* Instance(FGameplayAbilitySpecHandle Handle) const;
    UGastroTestObserver* Observe(UGastroAbilitySystemBindingComponent* Target);
    UEnhancedInputComponent* MakeInput(AActor* Owner);
    void Dispatch(UEnhancedInputComponent* Input, ETriggerEvent Event);
END_DEFINE_SPEC(FGastroFrameworkSpec)

FGameplayAbilitySpecHandle FGastroFrameworkSpec::Grant(TSubclassOf<UGastroGameplayAbility> Ability)
{
    Set->GrantedAbilities[0].Ability = Ability;
    Set->GiveToAbilitySystem(ASC, Receipt, Actor);
    return Receipt.GetAbilitySpecHandles().IsEmpty() ? FGameplayAbilitySpecHandle() : Receipt.GetAbilitySpecHandles().Last();
}

UGastroTestAbility* FGastroFrameworkSpec::Instance(FGameplayAbilitySpecHandle Handle) const
{
    const FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(Handle);
    return Spec ? Cast<UGastroTestAbility>(Spec->GetPrimaryInstance()) : nullptr;
}

UGastroTestObserver* FGastroFrameworkSpec::Observe(UGastroAbilitySystemBindingComponent* Target)
{
    UGastroTestObserver* Observer = NewObject<UGastroTestObserver>(Target);
    Target->OnAbilitySystemReady.AddDynamic(Observer, &UGastroTestObserver::Ready);
    Target->OnAbilitySystemUninitializing.AddDynamic(Observer, &UGastroTestObserver::Unbinding);
    return Observer;
}

UEnhancedInputComponent* FGastroFrameworkSpec::MakeInput(AActor* Owner)
{
    UEnhancedInputComponent* Input = NewObject<UEnhancedInputComponent>(Owner);
    Input->RegisterComponent();
    return Input;
}

void FGastroFrameworkSpec::Dispatch(UEnhancedInputComponent* Input, ETriggerEvent Event)
{
    // Exercise the real bound delegate, including its InputTag payload.
    for (const TUniquePtr<FEnhancedInputActionEventBinding>& Entry : Input->GetActionEventBindings())
    {
        if (Entry->GetAction() == Action && Entry->GetTriggerEvent() == Event)
        {
            Entry->Execute(FInputActionInstance(Action));
        }
    }
}

void FGastroFrameworkSpec::Define()
{
    // Test-only, in-memory tags. Editor modules must not declare FNativeGameplayTag globals.
    TestInput = UGameplayTagsManager::Get().AddNativeGameplayTag(TEXT("Consumer.Test.Input"));
    TestInputChild = UGameplayTagsManager::Get().AddNativeGameplayTag(TEXT("Consumer.Test.Input.Child"));
    TestOther = UGameplayTagsManager::Get().AddNativeGameplayTag(TEXT("Consumer.Test.Other"));
    BeforeEach([this]
    {
        UWorld::InitializationValues Values;
        Values.AllowAudioPlayback(false).CreateNavigation(false).CreateAISystem(false);
        World = UWorld::CreateWorld(EWorldType::Game, false, NAME_None, nullptr, true, ERHIFeatureLevel::Num, &Values);
        GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
        World->InitializeActorsForPlay(FURL());
        World->BeginPlay();
        // No GameMode is needed; explicitly dispatch the real actor BeginPlay lifecycle.
        World->GetWorldSettings()->NotifyBeginPlay();
        Actor = World->SpawnActor<AGastroTestActor>();
        ASC = Actor->ASC;
        Binding = Actor->Binding;
        Binding->InitializeAbilitySystem(ASC, Actor);
        Set = NewObject<UGastroAbilitySet>();
        FGastroAbilitySetEntry Entry;
        Entry.Ability = UGastroTestAbility::StaticClass();
        Entry.AbilityLevel = 3;
        Entry.InputTag = TestInput;
        Set->GrantedAbilities.Add(Entry);
        Action = NewObject<UInputAction>();
        Action->ValueType = EInputActionValueType::Boolean;
        Config = NewObject<UGastroInputConfig>();
        FGastroAbilityInputAction Mapping;
        Mapping.InputAction = Action;
        Mapping.InputTag = TestInput;
        Config->AbilityInputActions.Add(Mapping);
        Receipt = FGastroAbilitySetGrantedHandles();
    });

    AfterEach([this]
    {
        Receipt.TakeFromAbilitySystem();
        World->EndPlay(EEndPlayReason::Quit);
        World->DestroyWorld(false);
        GEngine->DestroyWorldContext(World);
        World = nullptr;
    });

    Describe("AbilitySet", [this]
    {
        It("grants configured level source and consumer-defined input tag without activation", [this]
        {
            const auto Handle = Grant();
            const auto* Spec = ASC->FindAbilitySpecFromHandle(Handle);
            TestNotNull("Spec exists", Spec);
            if (!Spec) return;
            TestEqual("Level", Spec->Level, 3);
            TestTrue("Source", Spec->SourceObject == Actor);
            TestTrue("Consumer tag", Spec->GetDynamicSpecSourceTags().HasTagExact(TestInput));
            TestFalse("No implicit activation", Spec->IsActive());
        });
        It("allows abilities without input tags", [this]
        {
            Set->GrantedAbilities[0].InputTag = FGameplayTag();
            const auto Handle = Grant();
            TestTrue("No input tag", ASC->FindAbilitySpecFromHandle(Handle)->GetDynamicSpecSourceTags().IsEmpty());
            TestTrue("Explicit activation", ASC->TryActivateAbility(Handle));
        });
        It("supports empty sets and repeated independent grants", [this]
        {
            const auto First = Grant();
            const auto Second = Grant();
            TestTrue("Independent specs", First != Second);
            TestEqual("Receipt accumulates", Receipt.GetAbilitySpecHandles().Num(), 2);
            Set->GrantedAbilities.Empty();
            TestTrue("Empty set", Set->GiveToAbilitySystem(ASC, Receipt));
        });
        It("rejects null ASCs", [this]
        {
            TestFalse("Null ASC", Set->GiveToAbilitySystem(nullptr, Receipt));
            TestTrue("Unchanged receipt", Receipt.IsEmpty());
        });
        It("validates the entire set before any grant", [this]
        {
            auto Invalid = Set->GrantedAbilities[0];
            Invalid.Ability = nullptr;
            Set->GrantedAbilities.Add(Invalid);
            AddExpectedError("contains an invalid ability", EAutomationExpectedErrorFlags::Contains, 1);
            TestFalse("Rejected", Set->GiveToAbilitySystem(ASC, Receipt));
            TestEqual("No partial grant", ASC->GetActivatableAbilities().Num(), 0);
        });
        It("rejects abstract ability classes and non-positive levels", [this]
        {
            Set->GrantedAbilities[0].Ability = UGastroGameplayAbility::StaticClass();
            TestFalse("Abstract class", Set->GrantedAbilities[0].IsValid());
            Set->GrantedAbilities[0].Ability = UGastroTestAbility::StaticClass();
            Set->GrantedAbilities[0].AbilityLevel = 0;
            TestFalse("Zero level", Set->GrantedAbilities[0].IsValid());
            Set->GrantedAbilities[0].AbilityLevel = -1;
            TestFalse("Negative level", Set->GrantedAbilities[0].IsValid());
        });
        It("rejects cross-ASC receipt reuse without altering either ASC", [this]
        {
            Grant();
            auto* Other = World->SpawnActor<AGastroTestActor>();
            Other->Binding->InitializeAbilitySystem(Other->ASC, Other);
            AddExpectedError("cannot be shared between different ASCs", EAutomationExpectedErrorFlags::Contains, 1);
            TestFalse("Rejected", Set->GiveToAbilitySystem(Other->ASC, Receipt));
            TestEqual("First preserved", ASC->GetActivatableAbilities().Num(), 1);
            TestEqual("Other unchanged", Other->ASC->GetActivatableAbilities().Num(), 0);
        });
        It("rejects non-authoritative grants and removals", [this]
        {
            const auto Handle = Grant();
            Actor->SetRole(ROLE_SimulatedProxy);
            ASC->CacheIsNetSimulated();
            FGastroAbilitySetGrantedHandles ClientReceipt;
            TestFalse("No client grant", Set->GiveToAbilitySystem(ASC, ClientReceipt));
            Receipt.TakeFromAbilitySystem();
            TestFalse("Receipt retained until authority can revoke", Receipt.IsEmpty());
            TestNotNull("Spec retained", ASC->FindAbilitySpecFromHandle(Handle));
            Actor->SetRole(ROLE_Authority);
            ASC->CacheIsNetSimulated();
        });
        It("removes only its grant including an uncancelable active ability", [this]
        {
            const auto First = Grant();
            FGastroAbilitySetGrantedHandles Other;
            Set->GiveToAbilitySystem(ASC, Other);
            const auto OtherHandle = Other.GetAbilitySpecHandles()[0];
            ASC->TryActivateAbility(First);
            Instance(First)->SetCanBeCanceled(false);
            Receipt.TakeFromAbilitySystem();
            TestNull("Revoked spec", ASC->FindAbilitySpecFromHandle(First));
            TestNotNull("Other grant preserved", ASC->FindAbilitySpecFromHandle(OtherHandle));
            Receipt.TakeFromAbilitySystem();
            Other.TakeFromAbilitySystem();
        });
        It("clears receipt before ending callbacks reenter revoke", [this]
        {
            const auto Handle = Grant();
            ASC->TryActivateAbility(Handle);
            bool bSawEmpty = false;
            Instance(Handle)->OnEnding = [this, &bSawEmpty] { bSawEmpty = Receipt.IsEmpty(); Receipt.TakeFromAbilitySystem(); };
            Receipt.TakeFromAbilitySystem();
            TestTrue("Empty before callback", bSawEmpty);
            TestNull("Removed", ASC->FindAbilitySpecFromHandle(Handle));
        });
        It("tolerates already removed specs and a destroyed ASC", [this]
        {
            ASC->ClearAbility(Grant());
            Receipt.TakeFromAbilitySystem();
            Grant();
            Actor->Destroy();
            Receipt.TakeFromAbilitySystem();
            TestTrue("Cleared", Receipt.IsEmpty());
        });
        It("reports invalid data in editor validation", [this]
        {
            Set->GrantedAbilities[0].AbilityLevel = 0;
            FDataValidationContext Context;
            TestTrue("Invalid", Set->IsDataValid(Context) == EDataValidationResult::Invalid);
            TestTrue("Error reported", Context.GetNumErrors() > 0);
        });
    });

    Describe("AbilitySystem", [this]
    {
        It("routes exact input tags and rejects parent or unrelated tags", [this]
        {
            Set->GrantedAbilities[0].InputTag = TestInputChild;
            const auto Handle = Grant();
            ASC->AbilityInputTagPressed(TestInput);
            ASC->AbilityInputTagPressed(TestOther);
            ASC->AbilityInputTagPressed(FGameplayTag());
            TestFalse("No hierarchical match", ASC->FindAbilitySpecFromHandle(Handle)->IsActive());
            ASC->AbilityInputTagPressed(TestInputChild);
            TestTrue("Exact match", ASC->FindAbilitySpecFromHandle(Handle)->IsActive());
        });
        It("routes one input to multiple specs and forwards subsequent press and release", [this]
        {
            const auto First = Grant();
            const auto Second = Grant();
            ASC->AbilityInputTagPressed(TestInput);
            TestEqual("First activation", Instance(First)->Activations, 1);
            TestEqual("Second activation", Instance(Second)->Activations, 1);
            ASC->AbilityInputTagReleased(TestInput);
            ASC->AbilityInputTagPressed(TestInput);
            TestEqual("Subsequent press", Instance(First)->Presses, 1);
            TestEqual("Release", Instance(First)->Releases, 1);
            TestEqual("No reactivation", Instance(First)->Activations, 1);
        });
        It("notifies actual GAS input tasks using the active instance prediction key", [this]
        {
            const auto Handle = Grant();
            ASC->TryActivateAbility(Handle);
            auto* Ability = Instance(Handle);
            Ability->StartInputTasks();
            ASC->AbilityInputTagPressed(TestInput);
            ASC->AbilityInputTagReleased(TestInput);
            TestEqual("WaitInputPress", Ability->PressTaskEvents, 1);
            TestEqual("WaitInputRelease", Ability->ReleaseTaskEvents, 1);
        });
        It("ends input tasks on avatar exit even when ability cannot be canceled", [this]
        {
            const auto Handle = Grant();
            ASC->TryActivateAbility(Handle);
            auto* Ability = Instance(Handle);
            Ability->StartInputTasks();
            Ability->SetCanBeCanceled(false);
            Binding->UninitializeAbilitySystem();
            TestFalse("Ability ended", ASC->FindAbilitySpecFromHandle(Handle)->IsActive());
            TestTrue("Press task finished", Ability->PressTask->IsFinished());
            TestTrue("Release task finished", Ability->ReleaseTask->IsFinished());
            TestFalse("Input flag cleared", ASC->FindAbilitySpecFromHandle(Handle)->InputPressed);
        });
        It("clears inactive ability avatar caches as well", [this]
        {
            const auto Handle = Grant();
            ASC->TryActivateAbility(Handle);
            auto* Ability = Instance(Handle);
            Ability->Finish();
            Binding->UninitializeAbilitySystem();
            TestEqual("Exit callback", Ability->AvatarExits, 1);
            TestFalse("Old avatar released", Ability->CachedAvatar.IsValid());
        });
        It("allows explicit persistent abilities to survive and clear old avatar references", [this]
        {
            const auto Handle = Grant(UGastroTestPersistentAbility::StaticClass());
            ASC->TryActivateAbility(Handle);
            auto* Ability = Instance(Handle);
            Binding->UninitializeAbilitySystem();
            TestTrue("Still active", ASC->FindAbilitySpecFromHandle(Handle)->IsActive());
            TestFalse("Cache cleared", Ability->CachedAvatar.IsValid());
            TestEqual("Exit callback", Ability->AvatarExits, 1);
            TestFalse("Cannot start another activation without avatar", ASC->TryActivateAbility(Grant()));
            Receipt.TakeFromAbilitySystem();
            TestNull("Receipt overrides persistent policy", ASC->FindAbilitySpecFromHandle(Handle));
        });
        It("does not start abilities before binding is ready", [this]
        {
            const auto Handle = Grant();
            Binding->UninitializeAbilitySystem();
            ASC->AbilityInputTagPressed(TestInput);
            TestFalse("No input activation", ASC->FindAbilitySpecFromHandle(Handle)->IsActive());
            TestFalse("No code activation", ASC->TryActivateAbility(Handle));
        });
    });

    Describe("Binding", [this]
    {
        It("is idempotent and refreshes actor info without duplicating grants or notifications", [this]
        {
            Binding->UninitializeAbilitySystem();
            Binding->AvatarAbilitySets.Add(Set);
            auto* Observer = Observe(Binding);
            TestTrue("First bind", Binding->InitializeAbilitySystem(ASC, Actor));
            const uint64 Generation = ASC->GetBindingGeneration();
            TestTrue("Repeated bind", Binding->InitializeAbilitySystem(ASC, Actor));
            TestEqual("One grant", ASC->GetActivatableAbilities().Num(), 1);
            TestEqual("One notification", Observer->ReadyCount, 1);
            TestEqual("Same generation", ASC->GetBindingGeneration(), Generation);
            TestTrue("Owner", ASC->AbilityActorInfo->OwnerActor == Actor);
            TestTrue("Avatar", ASC->AbilityActorInfo->AvatarActor == Actor);
        });
        It("retries when dependencies become available and rejects mismatched owners", [this]
        {
            Binding->UninitializeAbilitySystem();
            TestFalse("Missing ASC", Binding->InitializeAbilitySystem(nullptr, Actor));
            TestFalse("Missing owner", Binding->InitializeAbilitySystem(ASC, nullptr));
            auto* Other = World->SpawnActor<AActor>();
            TestFalse("Incorrect owner", Binding->InitializeAbilitySystem(ASC, Other));
            TestTrue("Retry", Binding->InitializeAbilitySystem(ASC, Actor));
        });
        It("moves an external ASC and ignores stale old-Pawn cleanup", [this]
        {
            auto* NewAvatar = World->SpawnActor<AGastroTestActor>();
            auto* OldObserver = Observe(Binding);
            TestTrue("New avatar", NewAvatar->Binding->InitializeAbilitySystem(ASC, Actor));
            TestFalse("Old not ready", Binding->IsAbilitySystemReady());
            TestEqual("Old notified", OldObserver->UnbindingCount, 1);
            Binding->UninitializeAbilitySystem();
            TestTrue("New binding remains", ASC->GetAvatarBinding() == NewAvatar->Binding);
            TestTrue("New actor info", ASC->GetAvatarActor() == NewAvatar);
            Actor->Binding->DestroyComponent();
            TestTrue("Still new avatar", ASC->GetAvatarActor() == NewAvatar);
        });
        It("releases avatar grants while keeping owner grants attributes and effects", [this]
        {
            auto* Owner = World->SpawnActor<AGastroTestActor>();
            Owner->ASC->OwnerAbilitySets.Add(Set);
            Owner->ASC->OwnerInitializationEffect = UGastroTestInitEffect::StaticClass();
            Binding->UninitializeAbilitySystem();
            Binding->AvatarAbilitySets.Add(Set);
            Binding->InitializeAbilitySystem(Owner->ASC, Owner);
            const auto Effect = Owner->ASC->ApplyGameplayEffectToSelf(GetDefault<UGastroTestDurationEffect>(), 1, Owner->ASC->MakeEffectContext());
            TestEqual("Owner plus avatar", Owner->ASC->GetActivatableAbilities().Num(), 2);
            Binding->UninitializeAbilitySystem();
            TestEqual("Owner grant remains", Owner->ASC->GetActivatableAbilities().Num(), 1);
            TestEqual("Value preserved", Owner->Attributes->GetValue(), 25.0f);
            TestNotNull("Effect preserved", Owner->ASC->GetActiveGameplayEffect(Effect));
            TestTrue("Owner remains", Owner->ASC->GetOwnerActor() == Owner);
            TestNull("Avatar cleared", Owner->ASC->GetAvatarActor());
        });
        It("handles recursive unbind and blocks rebind during cleanup", [this]
        {
            auto* Observer = Observe(Binding);
            bool bNestedBind = true;
            Observer->OnUnbinding = [this, &bNestedBind] {
                TestFalse("Not ready during callback", Binding->IsAbilitySystemReady());
                Binding->UninitializeAbilitySystem();
                bNestedBind = Binding->InitializeAbilitySystem(ASC, Actor);
            };
            Binding->UninitializeAbilitySystem();
            TestFalse("Rebind deferred", bNestedBind);
            TestEqual("Only one cleanup", Observer->UnbindingCount, 1);
            TestTrue("Later retry succeeds", Binding->InitializeAbilitySystem(ASC, Actor));
            Observer->OnUnbinding = nullptr;
        });
        It("ignores a nested ready notification that unbinds the association", [this]
        {
            Binding->UninitializeAbilitySystem();
            auto* Observer = Observe(Binding);
            Observer->OnReady = [this] { Binding->UninitializeAbilitySystem(); };
            TestFalse("No longer ready", Binding->InitializeAbilitySystem(ASC, Actor));
            TestFalse("Not ready", ASC->IsAvatarReady());
        });
        It("honors unbinding requested inside an attribute initialization callback", [this]
        {
            auto* Owner = World->SpawnActor<AGastroTestActor>();
            Owner->ASC->OwnerInitializationEffect = UGastroTestInitEffect::StaticClass();
            auto* Observer = Observe(Owner->Binding);
            Owner->ASC->GetGameplayAttributeValueChangeDelegate(UGastroTestAttributes::GetValueAttribute())
                .AddLambda([Owner](const FOnAttributeChangeData&) { Owner->Binding->UninitializeAbilitySystem(); });
            TestFalse("Initialization canceled", Owner->Binding->InitializeAbilitySystem(Owner->ASC, Owner));
            TestFalse("No stale ready state", Owner->ASC->IsAvatarReady());
            TestEqual("No ready notification", Observer->ReadyCount, 0);
            TestEqual("Owner initialization still executed once", Owner->Attributes->GetValue(), 25.0f);
            TestTrue("Subsequent bind", Owner->Binding->InitializeAbilitySystem(Owner->ASC, Owner));
            TestEqual("No second effect", Owner->Attributes->GetValue(), 25.0f);
        });
        It("cleans pawn-scoped grants on owner actor destruction", [this]
        {
            Binding->UninitializeAbilitySystem();
            Binding->AvatarAbilitySets.Add(Set);
            Binding->InitializeAbilitySystem(ASC, Actor);
            auto* Observer = Observe(Binding);
            Actor->Destroy();
            TestEqual("Cleanup notified", Observer->UnbindingCount, 1);
            TestEqual("No grant remains", ASC->GetActivatableAbilities().Num(), 0);
        });
        It("unbinds and revokes avatar grants when the binding component unregisters", [this]
        {
            Binding->UninitializeAbilitySystem();
            Binding->AvatarAbilitySets.Add(Set);
            Binding->InitializeAbilitySystem(ASC, Actor);
            Binding->UnregisterComponent();
            TestFalse("No ready binding", ASC->IsAvatarReady());
            TestNull("No old avatar", ASC->GetAvatarActor());
            TestEqual("Avatar grant removed", ASC->GetActivatableAbilities().Num(), 0);
        });
    });

    Describe("Attributes", [this]
    {
        It("registers consumer default subobjects and provides typed ASC access", [this]
        {
            TestTrue("Default attributes discovered", ASC->GetSet<UGastroTestAttributes>() == Actor->Attributes);
            TestTrue("Typed accessor", Actor->Attributes->GetGastroAbilitySystemComponent() == ASC);
        });
        It("applies the owner initialization effect once across repeated binds and avatar replacement", [this]
        {
            auto* Owner = World->SpawnActor<AGastroTestActor>();
            Owner->ASC->OwnerInitializationEffect = UGastroTestInitEffect::StaticClass();
            TestTrue("Initialized", Owner->Binding->InitializeAbilitySystem(Owner->ASC, Owner));
            TestEqual("Actual additive effect", Owner->Attributes->GetValue(), 25.0f);
            Owner->Binding->InitializeAbilitySystem(Owner->ASC, Owner);
            Binding->UninitializeAbilitySystem();
            Binding->InitializeAbilitySystem(Owner->ASC, Owner);
            TestEqual("Not reapplied", Owner->Attributes->GetValue(), 25.0f);
            TestTrue("Explicit initialized state", Owner->ASC->IsOwnerStateInitialized());
        });
        It("rejects non-Instant initialization effects before granting or changing attributes", [this]
        {
            auto* Owner = World->SpawnActor<AGastroTestActor>();
            Owner->ASC->OwnerInitializationEffect = UGastroTestDurationEffect::StaticClass();
            Owner->ASC->OwnerAbilitySets.Add(Set);
            AddExpectedError("initialization effect must be Instant", EAutomationExpectedErrorFlags::Contains, 1);
            TestFalse("Rejected", Owner->Binding->InitializeAbilitySystem(Owner->ASC, Owner));
            TestEqual("No grant", Owner->ASC->GetActivatableAbilities().Num(), 0);
            Owner->ASC->OwnerInitializationEffect = UGastroTestInitEffect::StaticClass();
            TestTrue("Retry corrected configuration", Owner->Binding->InitializeAbilitySystem(Owner->ASC, Owner));
            TestEqual("Applied once", Owner->Attributes->GetValue(), 25.0f);
        });
        It("validates all owner and avatar sets before startup grants", [this]
        {
            auto* Owner = World->SpawnActor<AGastroTestActor>();
            auto* Invalid = NewObject<UGastroAbilitySet>();
            Invalid->GrantedAbilities.AddDefaulted();
            Owner->ASC->OwnerAbilitySets = {Set, Invalid};
            AddExpectedError("invalid startup AbilitySet", EAutomationExpectedErrorFlags::Contains, 1);
            TestFalse("Rejected", Owner->Binding->InitializeAbilitySystem(Owner->ASC, Owner));
            TestEqual("No partial owner grant", Owner->ASC->GetActivatableAbilities().Num(), 0);
        });
        It("rejects an invalid avatar set before changing the current association", [this]
        {
            auto* Other = World->SpawnActor<AGastroTestActor>();
            Other->Binding->AvatarAbilitySets.Add(nullptr);
            AddExpectedError("invalid startup AbilitySet", EAutomationExpectedErrorFlags::Contains, 1);
            TestFalse("Invalid avatar configuration", Other->Binding->InitializeAbilitySystem(ASC, Actor));
            TestTrue("Old association retained", Binding->IsAbilitySystemReady());
            TestTrue("Old avatar retained", ASC->GetAvatarActor() == Actor);
        });
        It("does not initialize owner state on a simulated proxy", [this]
        {
            auto* Owner = World->SpawnActor<AGastroTestActor>();
            Owner->ASC->OwnerInitializationEffect = UGastroTestInitEffect::StaticClass();
            Owner->ASC->OwnerAbilitySets.Add(Set);
            Owner->SetRole(ROLE_SimulatedProxy);
            Owner->ASC->CacheIsNetSimulated();
            TestTrue("Client actor info can bind", Owner->Binding->InitializeAbilitySystem(Owner->ASC, Owner));
            TestEqual("No client initialization", Owner->Attributes->GetValue(), 0.0f);
            TestEqual("No client grant", Owner->ASC->GetActivatableAbilities().Num(), 0);
            TestFalse("Authority state not marked", Owner->ASC->IsOwnerStateInitialized());
            Owner->SetRole(ROLE_Authority);
            Owner->ASC->CacheIsNetSimulated();
        });
    });

    Describe("Input", [this]
    {
        It("validates Boolean actions valid tags and duplicate mappings", [this]
        {
            TestTrue("Valid mapping", Config->AbilityInputActions[0].IsValid());
            Action->ValueType = EInputActionValueType::Axis1D;
            TestFalse("Only Boolean", Config->AbilityInputActions[0].IsValid());
            Action->ValueType = EInputActionValueType::Boolean;
            const auto Duplicate = Config->AbilityInputActions[0];
            Config->AbilityInputActions.Add(Duplicate);
            FDataValidationContext Context;
            TestTrue("Duplicates invalid", Config->IsDataValid(Context) == EDataValidationResult::Invalid);
            Config->AbilityInputActions[0].InputTag = FGameplayTag();
            TestFalse("Empty tag", Config->AbilityInputActions[0].IsValid());
            Config->AbilityInputActions[0].InputAction = nullptr;
            TestFalse("Null action", Config->AbilityInputActions[0].IsValid());
        });
        It("routes real action delegates and Completed or Canceled releases", [this]
        {
            const auto Handle = Grant();
            auto* Route = NewObject<UGastroAbilityInputComponent>(Actor);
            Route->RegisterComponent();
            Route->SetAbilitySystemComponent(ASC);
            auto* Input = MakeInput(Actor);
            Route->BindInputs(Input, Config);
            Dispatch(Input, ETriggerEvent::Started);
            TestTrue("Activated", ASC->FindAbilitySpecFromHandle(Handle)->IsActive());
            Dispatch(Input, ETriggerEvent::Completed);
            TestFalse("Completed released", ASC->FindAbilitySpecFromHandle(Handle)->InputPressed);
            Dispatch(Input, ETriggerEvent::Started);
            Dispatch(Input, ETriggerEvent::Canceled);
            TestEqual("Both releases reached instance", Instance(Handle)->Releases, 2);
        });
        It("releases input when replacing configuration and does not duplicate bindings", [this]
        {
            const auto Handle = Grant();
            auto* Route = NewObject<UGastroAbilityInputComponent>(Actor);
            Route->SetAbilitySystemComponent(ASC);
            auto* Input = MakeInput(Actor);
            Route->BindInputs(Input, Config);
            Dispatch(Input, ETriggerEvent::Started);
            Route->BindInputs(Input, NewObject<UGastroInputConfig>());
            TestFalse("Released old config", ASC->FindAbilitySpecFromHandle(Handle)->InputPressed);
            TestEqual("Removed old bindings", Input->GetActionEventBindings().Num(), 0);
            Route->BindInputs(Input, Config);
            Route->BindInputs(Input, Config);
            TestEqual("Exactly three bindings", Input->GetActionEventBindings().Num(), 3);
        });
        It("preserves unrelated bindings and supports replacing the input component", [this]
        {
            auto* Route = NewObject<UGastroAbilityInputComponent>(Actor);
            auto* Input = MakeInput(Actor);
            auto* OtherInput = MakeInput(Actor);
            auto* Unrelated = NewObject<UInputAction>();
            Input->BindActionValue(Unrelated);
            Route->BindInputs(Input, Config);
            Route->BindInputs(OtherInput, Config);
            TestEqual("Own old bindings removed", Input->GetActionEventBindings().Num(), 0);
            TestEqual("External binding retained", Input->GetActionValueBindings().Num(), 1);
            Route->UnbindInputs();
            TestEqual("New bindings removed", OtherInput->GetActionEventBindings().Num(), 0);
        });
        It("releases to the original ASC when its target changes", [this]
        {
            const auto Handle = Grant();
            auto* Route = NewObject<UGastroAbilityInputComponent>(Actor);
            auto* Input = MakeInput(Actor);
            Route->SetAbilitySystemComponent(ASC);
            Route->BindInputs(Input, Config);
            Dispatch(Input, ETriggerEvent::Started);
            auto* Other = World->SpawnActor<AGastroTestActor>();
            Other->Binding->InitializeAbilitySystem(Other->ASC, Other);
            Route->SetAbilitySystemComponent(Other->ASC);
            TestFalse("Old ASC released", ASC->FindAbilitySpecFromHandle(Handle)->InputPressed);
            Dispatch(Input, ETriggerEvent::Completed);
            TestEqual("Only one release", Instance(Handle)->Releases, 1);
            Route->UnbindInputs();
        });
        It("skips invalid duplicate mappings with precise diagnostics", [this]
        {
            auto* Route = NewObject<UGastroAbilityInputComponent>(Actor);
            auto* Input = MakeInput(Actor);
            const auto Duplicate = Config->AbilityInputActions[0];
            Config->AbilityInputActions.Add(Duplicate);
            Config->AbilityInputActions.AddDefaulted();
            AddExpectedError("skipping invalid or duplicate", EAutomationExpectedErrorFlags::Contains, 2);
            Route->BindInputs(Input, Config);
            TestEqual("Only valid mapping bound", Input->GetActionEventBindings().Num(), 3);
            Route->UnbindInputs();
        });
        It("cleans bindings on component destruction", [this]
        {
            const auto Handle = Grant();
            auto* Route = NewObject<UGastroAbilityInputComponent>(Actor);
            Route->RegisterComponent();
            Route->SetAbilitySystemComponent(ASC);
            auto* Input = MakeInput(Actor);
            Route->BindInputs(Input, Config);
            Dispatch(Input, ETriggerEvent::Started);
            Route->DestroyComponent();
            TestFalse("Released", ASC->FindAbilitySpecFromHandle(Handle)->InputPressed);
            TestEqual("Removed", Input->GetActionEventBindings().Num(), 0);
        });
        It("observes an explicitly injected ASC that becomes ready later across repeated avatar lifetimes", [this]
        {
            const auto Handle = Grant();
            Binding->UninitializeAbilitySystem();
            auto* Route = NewObject<UGastroAbilityInputComponent>(Actor);
            Route->RegisterComponent();
            Route->SetAbilitySystemComponent(ASC);
            auto* Input = MakeInput(Actor);
            Route->BindInputs(Input, Config);
            Binding->InitializeAbilitySystem(ASC, Actor);
            Dispatch(Input, ETriggerEvent::Started);
            Binding->UninitializeAbilitySystem();
            Binding->InitializeAbilitySystem(ASC, Actor);
            Dispatch(Input, ETriggerEvent::Started);
            TestTrue("New press accepted without old completion", ASC->FindAbilitySpecFromHandle(Handle)->InputPressed);
            TestEqual("New activation", Instance(Handle)->Activations, 2);
            Route->DestroyComponent();
        });
    });

    Describe("Composition", [this]
    {
        It("integrates ordinary UE Character Controller and PlayerState including delayed readiness and pawn switching", [this]
        {
            auto* Controller = World->SpawnActor<AGastroConsumerController>();
            Controller->SetAsLocalPlayerController();
            auto* State = World->SpawnActor<AGastroConsumerPlayerState>();
            State->SetOwner(Controller);
            State->ASC->OwnerInitializationEffect = UGastroTestInitEffect::StaticClass();
            State->ASC->OwnerAbilitySets.Add(Set);
            auto* First = World->SpawnActor<AGastroConsumerExternalCharacter>();
            auto* Second = World->SpawnActor<AGastroConsumerExternalCharacter>();
            Controller->Possess(First);
            auto* Input = MakeInput(Controller);
            Controller->AbilityInput->BindInputs(Input, Config);
            Dispatch(Input, ETriggerEvent::Started);
            TestEqual("No grant before ready", State->ASC->GetActivatableAbilities().Num(), 0);
            First->Binding->InitializeAbilitySystem(State->ASC, State);
            const auto Handle = State->ASC->GetActivatableAbilities()[0].Handle;
            Dispatch(Input, ETriggerEvent::Started);
            TestTrue("Input activated after ready", State->ASC->FindAbilitySpecFromHandle(Handle)->IsActive());
            Controller->Possess(Second);
            TestFalse("Old held input released", State->ASC->FindAbilitySpecFromHandle(Handle)->InputPressed);
            Second->Binding->InitializeAbilitySystem(State->ASC, State);
            First->Destroy();
            TestTrue("New avatar retained", State->ASC->GetAvatarActor() == Second);
            TestEqual("Owner grant not duplicated", State->ASC->GetActivatableAbilities().Num(), 1);
            TestEqual("Owner value retained", State->Attributes->GetValue(), 25.0f);
            Dispatch(Input, ETriggerEvent::Started);
            TestTrue("New pawn receives input", State->ASC->FindAbilitySpecFromHandle(Handle)->IsActive());
            Second->Binding->UninitializeAbilitySystem();
            TestFalse("Input released during detach", State->ASC->FindAbilitySpecFromHandle(Handle)->InputPressed);
            Controller->UnPossess();
        });
        It("supports owned ASC on an ordinary Character without clearing state on unpossess", [this]
        {
            auto* Controller = World->SpawnActor<AGastroConsumerController>();
            Controller->SetAsLocalPlayerController();
            auto* Pawn = World->SpawnActor<AGastroConsumerCharacter>();
            Pawn->ASC->OwnerInitializationEffect = UGastroTestInitEffect::StaticClass();
            Pawn->Binding->AvatarAbilitySets.Add(Set);
            Controller->Possess(Pawn);
            Pawn->Binding->InitializeAbilitySystem(Pawn->ASC, Pawn);
            auto* Input = MakeInput(Controller);
            Controller->AbilityInput->BindInputs(Input, Config);
            Dispatch(Input, ETriggerEvent::Started);
            Controller->UnPossess();
            TestTrue("Body remains ready", Pawn->Binding->IsAbilitySystemReady());
            TestEqual("Body grants remain", Pawn->ASC->GetActivatableAbilities().Num(), 1);
            TestFalse("Input released", Pawn->ASC->GetActivatableAbilities()[0].InputPressed);
            TestEqual("Custom attributes", Pawn->Attributes->GetValue(), 25.0f);
        });
        It("supports explicit input injection and restoring automatic pawn following", [this]
        {
            const auto Handle = Grant();
            auto* Controller = World->SpawnActor<AGastroConsumerController>();
            Controller->SetAsLocalPlayerController();
            auto* Pawn = World->SpawnActor<AGastroConsumerCharacter>();
            Controller->AbilityInput->SetAbilitySystemComponent(ASC);
            Controller->Possess(Pawn);
            auto* Input = MakeInput(Controller);
            Controller->AbilityInput->BindInputs(Input, Config);
            Dispatch(Input, ETriggerEvent::Started);
            TestTrue("Explicit target preserved", ASC->FindAbilitySpecFromHandle(Handle)->IsActive());
            Controller->AbilityInput->FollowControllerPawn();
            TestFalse("Old target released", ASC->FindAbilitySpecFromHandle(Handle)->InputPressed);
        });
    });

    Describe("Convenience", [this]
    {
        It("keeps the base abstract and assembles an owned ASC through the public component", [this]
        {
            TestTrue("Abstract base", AGastroBaseCharacter::StaticClass()->HasAnyClassFlags(CLASS_Abstract));
            auto* Pawn = World->SpawnActor<AGastroCharacterWithOwnedASC>();
            TestNotNull("ASC", Pawn->GetAbilitySystemComponent());
            TestTrue("Public component ready", Pawn->GetAbilitySystemBinding()->IsAbilitySystemReady());
            auto* Owned = CastChecked<UGastroAbilitySystemComponent>(Pawn->GetAbilitySystemComponent());
            TestTrue("Owner is self", Owned->GetOwnerActor() == Pawn);
            TestTrue("Avatar is self", Owned->GetAvatarActor() == Pawn);
        });
        It("resolves external ASC from any interface-enabled PlayerState", [this]
        {
            auto* Pawn = World->SpawnActor<AGastroCharacterWithExternalASC>();
            TestNull("No local ASC", Pawn->GetAbilitySystemComponent());
            auto* State = World->SpawnActor<AGastroConsumerPlayerState>();
            Pawn->SetPlayerState(State);
            Pawn->OnRep_PlayerState();
            TestTrue("Consumer PlayerState used", Pawn->GetAbilitySystemComponent() == State->ASC);
            TestTrue("Ready", Pawn->GetAbilitySystemBinding()->IsAbilitySystemReady());
            TArray<UGastroAbilitySystemComponent*> LocalASCs;
            Pawn->GetComponents(LocalASCs);
            TestEqual("No redundant local ASC", LocalASCs.Num(), 0);
        });
        It("accepts explicit non-PlayerState external owners and can restore default resolution", [this]
        {
            auto* Pawn = World->SpawnActor<AGastroCharacterWithExternalASC>();
            Pawn->SetExternalAbilitySystemComponent(ASC);
            TestTrue("External actor source", Pawn->GetAbilitySystemComponent() == ASC);
            Pawn->SetExternalAbilitySystemComponent(nullptr);
            TestFalse("No source ready", Pawn->GetAbilitySystemBinding()->IsAbilitySystemReady());
            TestNull("External owner avatar detached", ASC->GetAvatarActor());
        });
        It("does not let a late old-Pawn replication callback steal the PlayerState ASC", [this]
        {
            auto* First = World->SpawnActor<AGastroCharacterWithExternalASC>();
            auto* Second = World->SpawnActor<AGastroCharacterWithExternalASC>();
            auto* State = World->SpawnActor<AGastroConsumerPlayerState>();
            First->SetPlayerState(State);
            First->OnRep_PlayerState();
            Second->SetPlayerState(State);
            Second->OnRep_PlayerState();
            First->OnRep_Controller();
            TestTrue("Current pawn retained", State->ASC->GetAvatarActor() == Second);
            TestFalse("Old pawn stays detached", First->GetAbilitySystemBinding()->IsAbilitySystemReady());
        });
        It("provides an optional PlayerState ASC without built-in attributes", [this]
        {
            auto* State = World->SpawnActor<AGastroPlayerState>();
            auto* Owned = Cast<UGastroAbilitySystemComponent>(State->GetAbilitySystemComponent());
            TestNotNull("ASC", Owned);
            if (Owned) TestEqual("No preset attributes", Owned->GetSpawnedAttributes().Num(), 0);
        });
    });
}

#endif
