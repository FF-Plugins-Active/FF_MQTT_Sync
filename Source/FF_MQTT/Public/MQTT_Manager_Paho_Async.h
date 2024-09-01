// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

// Custom Includes.
#include "MQTT_Manager_Paho_Sync.h"

#include "MQTT_Manager_Paho_Async.generated.h"

UCLASS()
class FF_MQTT_API AMQTT_Manager_Paho_Async : public AActor
{
	GENERATED_BODY()
	
private:	

	MQTTAsync Client = nullptr;
	MQTTAsync_connectOptions Connection_Options;
	MQTTAsync_SSLOptions SSL_Options;
	FString ClientId;

protected:

	// Called when the game starts or when spawned.
	virtual void BeginPlay() override;

	// Called when the game end or when destroyed.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	

	// Sets default values for this actor's properties.
	AMQTT_Manager_Paho_Async();

	// Called every frame.
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	virtual void MQTT_Async_Destroy();

};
