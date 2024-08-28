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
	if (this->Client)
	{
		this->MQTT_Destroy();
	}

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

void AMQTT_Manager_Paho::MQTT_Init(FDelegate_Paho_Connection DelegateConnection, FString In_Username, FString In_Pass, FString In_ClientId, FString In_Address, int32 In_Port, int32 KeepAliveInterval, bool bUseV5)
{
	this->bIsV5 = bUseV5;
	FJsonObjectWrapper TempCode;

	if (this->Client)
	{
		TempCode.JsonObject->SetStringField("ClassName", "AMQTT_Manager_Paho");
		TempCode.JsonObject->SetStringField("FunctionName", "MQTT_Init");
		TempCode.JsonObject->SetStringField("Description", "Client already initialized.");
		TempCode.JsonObject->SetStringField("AdditionalInfo", "");

		DelegateConnection.ExecuteIfBound(false, TempCode);
		return;
	}

	AsyncTask(ENamedThreads::AnyNormalThreadNormalTask, [this, DelegateConnection, TempCode, In_Username, In_Pass, In_ClientId, In_Address, In_Port, KeepAliveInterval]()
		{
			MQTTClient TempClient = nullptr;
			int RetVal = MQTTClient_create(&TempClient, TCHAR_TO_UTF8(*In_Address), TCHAR_TO_UTF8(*In_ClientId), MQTTCLIENT_PERSISTENCE_NONE, NULL);

			if (RetVal != MQTTCLIENT_SUCCESS)
			{
				TempCode.JsonObject->SetStringField("ClassName", "AMQTT_Manager_Paho");
				TempCode.JsonObject->SetStringField("FunctionName", "MQTT_Init");
				TempCode.JsonObject->SetStringField("Description", "There was a problem while creating client. Code : " + (FString)FString::FromInt(RetVal));
				TempCode.JsonObject->SetStringField("AdditionalInfo", "");

				MQTTClient_destroy(&TempClient);

				AsyncTask(ENamedThreads::GameThread, [DelegateConnection, TempCode]()
					{
						DelegateConnection.ExecuteIfBound(false, TempCode);
					}
				);

				return;
			}

			RetVal = MQTTClient_setCallbacks(TempClient, this, AMQTT_Manager_Paho::ConnectionLost, AMQTT_Manager_Paho::MessageArrived, AMQTT_Manager_Paho::MessageDelivered);

			if (RetVal != MQTTCLIENT_SUCCESS)
			{
				TempCode.JsonObject->SetStringField("ClassName", "AMQTT_Manager_Paho");
				TempCode.JsonObject->SetStringField("FunctionName", "MQTT_Init");
				TempCode.JsonObject->SetStringField("Description", "There was a problem while setting callbacks. Code : " + (FString)FString::FromInt(RetVal));
				TempCode.JsonObject->SetStringField("AdditionalInfo", "");

				MQTTClient_destroy(&TempClient);

				AsyncTask(ENamedThreads::GameThread, [DelegateConnection, TempCode]()
					{
						DelegateConnection.ExecuteIfBound(false, TempCode);
					}
				);

				return;
			}

			this->Connection_Options = MQTTClient_connectOptions_initializer;
			this->Connection_Options.keepAliveInterval = KeepAliveInterval;
			this->Connection_Options.cleansession = 1;
			this->Connection_Options.username = TCHAR_TO_UTF8(*In_Username);
			this->Connection_Options.password = TCHAR_TO_UTF8(*In_Pass);
			this->Connection_Options.MQTTVersion = this->bIsV5 ? MQTTVERSION_5 : MQTTVERSION_DEFAULT;

			RetVal = MQTTClient_connect(TempClient, &this->Connection_Options);

			if (RetVal != MQTTCLIENT_SUCCESS)
			{
				TempCode.JsonObject->SetStringField("ClassName", "AMQTT_Manager_Paho");
				TempCode.JsonObject->SetStringField("FunctionName", "MQTT_Init");
				TempCode.JsonObject->SetStringField("Description", "There was a problem while making connection. Code : " + (FString)FString::FromInt(RetVal));
				TempCode.JsonObject->SetStringField("AdditionalInfo", "");

				MQTTClient_destroy(&TempClient);

				AsyncTask(ENamedThreads::GameThread, [DelegateConnection, TempCode]()
					{
						DelegateConnection.ExecuteIfBound(false, TempCode);
					}
				);

				return;
			}

			TempCode.JsonObject->SetStringField("ClassName", "AMQTT_Manager_Paho");
			TempCode.JsonObject->SetStringField("FunctionName", "MQTT_Init");
			TempCode.JsonObject->SetStringField("Description", "Connection successful.");
			TempCode.JsonObject->SetStringField("AdditionalInfo", "");

			AsyncTask(ENamedThreads::GameThread, [this, DelegateConnection, TempCode, TempClient]()
				{
					this->Client = TempClient;
					DelegateConnection.ExecuteIfBound(false, TempCode);
				}
			);
		}
	);
}

void AMQTT_Manager_Paho::MQTT_Destroy()
{
	if (this->Client)
	{
		return;
	}	

	if (MQTTClient_isConnected(this->Client))
	{
		if (this->bIsV5)
		{
			MQTTClient_disconnect5(this->Client, 10000, MQTTREASONCODE_NORMAL_DISCONNECTION, NULL);
		}

		else
		{
			MQTTClient_disconnect(this->Client, 10000);
		}	
	}

	MQTTClient_destroy(&this->Client);
}
