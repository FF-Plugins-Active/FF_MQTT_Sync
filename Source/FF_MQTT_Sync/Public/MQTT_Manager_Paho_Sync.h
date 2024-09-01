// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

// Custom Includes.
#include "MQTT_Sync_Includes.h"

#include "MQTT_Manager_Paho_Sync.generated.h"

UCLASS()
class FF_MQTT_SYNC_API AMQTT_Manager_Paho_Sync : public AActor
{
	GENERATED_BODY()
	
private:

	MQTTClient Client = nullptr;
	MQTTClient_connectOptions Connection_Options;
	MQTTClient_SSLOptions SSL_Options;
	FPahoClientParams_Sync Client_Params;

#pragma region CALLBACKS
	static void MessageDelivered(void* CallbackContext, MQTTClient_deliveryToken In_DeliveryToken);
	static int MessageArrived(void* CallbackContext, char* TopicName, int TopicLenght, MQTTClient_message* Message);
	static void ConnectionLost(void* CallbackContext, char* Cause);
	virtual bool SetSSLParams(FString In_Protocol, FPahoClientParams_Sync In_Params);
#pragma endregion CALLBACKS

protected:
	
	// Called when the game starts or when spawned.
	virtual void BeginPlay() override;

	// Called when the game end or when destroyed.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	

	// Sets default values for this actor's properties.
	AMQTT_Manager_Paho_Sync();

	// Called every frame.
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|MQTT|Client|Paho C")
	FDelegate_Paho_Delivered_Sync Delegate_Delivered;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|MQTT|Client|Paho C")
	FDelegate_Paho_Arrived_Sync Delegate_Arrived;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|MQTT|Client|Paho C")
	FDelegate_Paho_Lost_Sync Delegate_Lost;

	UFUNCTION(BlueprintPure)
	virtual FPahoClientParams_Sync GetClientParams();

	UFUNCTION(BlueprintCallable)
	virtual void MQTT_Sync_Destroy();

	UFUNCTION(BlueprintCallable)
	virtual void MQTT_Sync_Init(FDelegate_Paho_Connection_Sync DelegateConnection, FPahoClientParams_Sync In_Params);

	UFUNCTION(BlueprintCallable)
	virtual bool MQTT_Sync_Publish(FJsonObjectWrapper& Out_Code, FString In_Topic, FString In_Payload, EMQTTQOS_Sync In_QoS = EMQTTQOS_Sync::QoS_0, int32 In_Retained = 0);

	UFUNCTION(BlueprintCallable)
	virtual bool MQTT_Sync_Subscribe(FJsonObjectWrapper& Out_Code, FString In_Topic, EMQTTQOS_Sync In_QoS = EMQTTQOS_Sync::QoS_0);

};
