//Fill out your copyright notice in the Description page of Project Settings.

#include "MQTT_Manager_Paho.h"

// Sets default values
AMQTT_Manager_Paho::AMQTT_Manager_Paho()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned.
void AMQTT_Manager_Paho::BeginPlay()
{
	Super::BeginPlay();
}

// Called when the game end or when destroyed.
void AMQTT_Manager_Paho::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

// Called every frame
void AMQTT_Manager_Paho::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

FString AMQTT_Manager_Paho::GetClientId()
{
	return this->ClientId;
}

#pragma region CALLBACKS
void AMQTT_Manager_Paho::MessageDelivered(void* CallbackContext, MQTTClient_deliveryToken In_DeliveryToken)
{
	AMQTT_Manager_Paho* Owner = Cast<AMQTT_Manager_Paho>((AMQTT_Manager_Paho*)CallbackContext);

	if (!Owner)
	{
		return;
	}

	const FString DeliveryTokenString = FString::FromInt(In_DeliveryToken);

	FPahoDelivered StrDelivered;
	StrDelivered.DeliveredToken = DeliveryTokenString;
	StrDelivered.bIsSuccessful = true;

	Owner->Delegate_Delivered.Broadcast(StrDelivered);

	MQTTClient_deliveryToken TempDeliveryToken = In_DeliveryToken;
}

int AMQTT_Manager_Paho::MessageArrived(void* CallbackContext, char* TopicName, int TopicLenght, MQTTClient_message* Message)
{
	AMQTT_Manager_Paho* Owner = Cast<AMQTT_Manager_Paho>((AMQTT_Manager_Paho*)CallbackContext);

	if (!Owner)
	{
		return 0;
	}

	FPahoArrived StrArrived;
	StrArrived.TopicName = UTF8_TO_TCHAR(TopicName);
	StrArrived.TopicLenght = TopicLenght;
	StrArrived.Message.AppendChars((char*)Message->payload, Message->payloadlen);

	Owner->Delegate_Arrived.Broadcast(StrArrived);

	MQTTClient_freeMessage(&Message);
	MQTTClient_free(TopicName);

	return 1;
}

void AMQTT_Manager_Paho::ConnectionLost(void* CallbackContext, char* Cause)
{
	AMQTT_Manager_Paho* Owner = Cast<AMQTT_Manager_Paho>((AMQTT_Manager_Paho*)CallbackContext);

	if (!Owner)
	{
		return;
	}

	const FString CauseString = UTF8_TO_TCHAR(Cause);
	Owner->Delegate_Lost.Broadcast(CauseString);
}
#pragma endregion CALLBACKS

bool AMQTT_Manager_Paho::MQTT_Init(FJsonObjectWrapper& Out_Code, FString In_ClientId, FString In_Address, int32 In_Port)
{
	if (this->Client)
	{
		Out_Code.JsonObject->SetStringField("ClassName", "AMQTT_Manager_Paho");
		Out_Code.JsonObject->SetStringField("FunctionName", "MQTT_Init");
		Out_Code.JsonObject->SetStringField("Description", "Client already initialized.");
		Out_Code.JsonObject->SetStringField("AdditionalInfo", "");

		return false;
	}

	this->Connection_Options = MQTTClient_connectOptions_initializer;

	int RetVal = MQTTClient_create(&this->Client, TCHAR_TO_UTF8(*In_Address), TCHAR_TO_UTF8(*In_ClientId), MQTTCLIENT_PERSISTENCE_NONE, NULL);

	if (RetVal != MQTTCLIENT_SUCCESS)
	{
		Out_Code.JsonObject->SetStringField("ClassName", "AMQTT_Manager_Paho");
		Out_Code.JsonObject->SetStringField("FunctionName", "MQTT_Init");
		Out_Code.JsonObject->SetStringField("Description", "There was a problem while creating client.");
		Out_Code.JsonObject->SetStringField("AdditionalInfo", "");

		return false;
	}

	MQTTClient_setCallbacks(this->Client, this, AMQTT_Manager_Paho::ConnectionLost, AMQTT_Manager_Paho::MessageArrived, AMQTT_Manager_Paho::MessageDelivered);

	return true;
}