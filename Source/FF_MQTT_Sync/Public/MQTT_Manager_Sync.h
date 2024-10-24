// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

// Custom Includes.
#include "MQTT_Includes.h"
#include "Paho_Sync_Includes.h"

#include "MQTT_Manager_Sync.generated.h"

UCLASS()
class FF_MQTT_SYNC_API AMQTT_Manager_Paho_Sync : public AActor
{
	GENERATED_BODY()
	
private:

	MQTTClient Client = nullptr;
	MQTTClient_connectOptions Connection_Options;
	MQTTClient_SSLOptions SSL_Options;
	FPahoClientParams Client_Params;

#pragma region CALLBACKS
	static void MessageDelivered(void* CallbackContext, MQTTClient_deliveryToken In_DeliveryToken);
	static int MessageArrived(void* CallbackContext, char* TopicName, int TopicLenght, MQTTClient_message* Message);
	static void ConnectionLost(void* CallbackContext, char* Cause);
	virtual bool SetSSLParams(FString In_Protocol, FPahoSslOptions In_Options);
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
	FDelegate_Paho_Delivered Delegate_Delivered;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|MQTT|Client|Paho C")
	FDelegate_Paho_Arrived Delegate_Arrived;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|MQTT|Client|Paho C")
	FDelegate_Paho_Lost Delegate_Lost;

	UFUNCTION(BlueprintPure)
	virtual FPahoClientParams GetClientParams();

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|MQTT|Client|Paho C", meta = (DisplayName = "MQTT Sync - Destroy", ToolTip = "Don't use it immediately after \"MQTT Async Init\" give some delay or better use it after \"Delegate OnConnect\"", KeyWords = "mqtt, sync, paho, client, destroy, close, disconnect"))
	virtual void MQTT_Sync_Destroy();

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|MQTT|Client|Paho C", meta = (DisplayName = "MQTT Sync - Init", ToolTip = "Don't use it immediately after \"MQTT Async Init\" give some delay or better use it after \"Delegate OnConnect\"", KeyWords = "mqtt, sync, paho, client, init, initialize, start, connect"))
	virtual void MQTT_Sync_Init(FDelegate_Paho_Connection DelegateConnection, FPahoClientParams In_Params);

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|MQTT|Client|Paho C", meta = (DisplayName = "MQTT Sync - Publish", ToolTip = "Don't use it immediately after \"MQTT Async Init\" give some delay or better use it after \"Delegate OnConnect\"", KeyWords = "mqtt, sync, paho, client, publish, publisher"))
	virtual bool MQTT_Sync_Publish(FJsonObjectWrapper& Out_Code, FString In_Topic, FString In_Payload, EMQTTQOS In_QoS = EMQTTQOS::QoS_0, int32 In_Retained = 0);

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|MQTT|Client|Paho C", meta = (DisplayName = "MQTT Sync - Subscribe", ToolTip = "Don't use it immediately after \"MQTT Async Init\" give some delay or better use it after \"Delegate OnConnect\"", KeyWords = "mqtt, sync, paho, client, subscribe, subscriber"))
	virtual bool MQTT_Sync_Subscribe(FJsonObjectWrapper& Out_Code, FString In_Topic, EMQTTQOS In_QoS = EMQTTQOS::QoS_0);

	UFUNCTION(BlueprintCallable, Category = "Frozen Forest|MQTT|Client|Paho C", meta = (DisplayName = "MQTT Sync - Unsubscribe", ToolTip = "Don't use it immediately after \"MQTT Async Init\" give some delay or better use it after \"Delegate OnConnect\"", KeyWords = "mqtt, sync, paho, client, unsubscribe, subscriber"))
	virtual bool MQTT_Sync_Unsubscribe(FJsonObjectWrapper& Out_Code, FString In_Topic);

};
