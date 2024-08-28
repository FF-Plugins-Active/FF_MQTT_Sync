// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

// Custom Includes.
#include "MQTT_Includes.h"

#include "MQTT_Manager_Paho.generated.h"

USTRUCT(BlueprintType)
struct FF_MQTT_API FPahoDelivered
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly)
	bool bIsSuccessful = false;

	UPROPERTY(BlueprintReadOnly)
	FString DeliveredToken;
};

USTRUCT(BlueprintType)
struct FF_MQTT_API FPahoArrived
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly)
	bool bIsSuccessful = false;

	UPROPERTY(BlueprintReadOnly)
	FString Message;

	UPROPERTY(BlueprintReadOnly)
	FString TopicName;

	UPROPERTY(BlueprintReadOnly)
	int32 TopicLenght = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegate_Paho_Delivered, FPahoDelivered, Out_Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegate_Paho_Arrived, FPahoArrived, Out_Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegate_Paho_Lost, FString, Out_Cause);

UCLASS()
class FF_MQTT_API AMQTT_Manager_Paho : public AActor
{
	GENERATED_BODY()
	
private:

	MQTTClient Client = nullptr;
	MQTTClient_connectOptions Connection_Options;
	FString ClientId;

#pragma region CALLBACKS
	static void MessageDelivered(void* CallbackContext, MQTTClient_deliveryToken In_DeliveryToken);
	static int MessageArrived(void* CallbackContext, char* TopicName, int TopicLenght, MQTTClient_message* Message);
	static void ConnectionLost(void* CallbackContext, char* Cause);
#pragma endregion CALLBACKS

protected:
	
	// Called when the game starts or when spawned.
	virtual void BeginPlay() override;

	// Called when the game end or when destroyed.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	

	// Sets default values for this actor's properties.
	AMQTT_Manager_Paho();

	// Called every frame.
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|MQTT|Client|Paho C")
	FDelegate_Paho_Delivered Delegate_Delivered;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|MQTT|Client|Paho C")
	FDelegate_Paho_Arrived Delegate_Arrived;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|MQTT|Client|Paho C")
	FDelegate_Paho_Lost Delegate_Lost;

	UFUNCTION(BlueprintPure)
	virtual FString GetClientId();

	UFUNCTION(BlueprintCallable)
	virtual bool MQTT_Init(FJsonObjectWrapper& Out_Code, FString In_ClientId, FString In_Address, int32 In_Port);

};
