// Fill out your copyright notice in the Description page of Project Settings.

#include "MQTT_Manager_Paho_Async.h"

// Sets default values
AMQTT_Manager_Paho_Async::AMQTT_Manager_Paho_Async()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned.
void AMQTT_Manager_Paho_Async::BeginPlay()
{
	Super::BeginPlay();
}

// Called when the game end or when destroyed.
void AMQTT_Manager_Paho_Async::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (this->Client)
	{
		this->MQTT_Async_Destroy();
	}

	Super::EndPlay(EndPlayReason);
}

// Called every frame.
void AMQTT_Manager_Paho_Async::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMQTT_Manager_Paho_Async::MQTT_Async_Destroy()
{

}