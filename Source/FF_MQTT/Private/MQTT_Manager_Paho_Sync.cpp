//Fill out your copyright notice in the Description page of Project Settings.

#include "MQTT_Manager_Paho_Sync.h"

// Sets default values
AMQTT_Manager_Paho_Sync::AMQTT_Manager_Paho_Sync()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned.
void AMQTT_Manager_Paho_Sync::BeginPlay()
{
	Super::BeginPlay();
}

// Called when the game end or when destroyed.
void AMQTT_Manager_Paho_Sync::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (this->Client)
	{
		this->MQTT_Sync_Destroy();
	}

	Super::EndPlay(EndPlayReason);
}

// Called every frame
void AMQTT_Manager_Paho_Sync::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

FString AMQTT_Manager_Paho_Sync::GetClientId()
{
	return this->ClientId;
}

#pragma region CALLBACKS
void AMQTT_Manager_Paho_Sync::MessageDelivered(void* CallbackContext, MQTTClient_deliveryToken In_DeliveryToken)
{
	AMQTT_Manager_Paho_Sync* Owner = Cast<AMQTT_Manager_Paho_Sync>((AMQTT_Manager_Paho_Sync*)CallbackContext);

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

int AMQTT_Manager_Paho_Sync::MessageArrived(void* CallbackContext, char* TopicName, int TopicLenght, MQTTClient_message* Message)
{
	AMQTT_Manager_Paho_Sync* Owner = Cast<AMQTT_Manager_Paho_Sync>((AMQTT_Manager_Paho_Sync*)CallbackContext);

	if (!Owner)
	{
		return 0;
	}

	FPahoArrived StrArrived;
	StrArrived.TopicName = UTF8_TO_TCHAR(TopicName);
	StrArrived.TopicLenght = TopicLenght;
	StrArrived.Message.AppendChars(UTF8_TO_TCHAR((char*)Message->payload), Message->payloadlen);

	Owner->Delegate_Arrived.Broadcast(StrArrived);

	MQTTClient_freeMessage(&Message);
	MQTTClient_free(TopicName);

	return 1;
}

void AMQTT_Manager_Paho_Sync::ConnectionLost(void* CallbackContext, char* Cause)
{
	AMQTT_Manager_Paho_Sync* Owner = Cast<AMQTT_Manager_Paho_Sync>((AMQTT_Manager_Paho_Sync*)CallbackContext);

	if (!Owner)
	{
		return;
	}

	const FString CauseString = UTF8_TO_TCHAR(Cause);
	Owner->Delegate_Lost.Broadcast(CauseString);
}
#pragma endregion CALLBACKS

void AMQTT_Manager_Paho_Sync::MQTT_Sync_Init(FDelegate_Paho_Connection DelegateConnection, FString In_Username, FString In_Pass, FString In_ClientId, FString In_Address, int32 KeepAliveInterval, EMQTTVERSION In_Version, bool bUseSSL, bool bUseWebSocket)
{
	FJsonObjectWrapper TempCode;
	TempCode.JsonObject->SetStringField("ClassName", "AMQTT_Manager_Paho_Sync");
	TempCode.JsonObject->SetStringField("FunctionName", "MQTT_Init");
	TempCode.JsonObject->SetStringField("AdditionalInfo", "");

	if (this->Client)
	{
		TempCode.JsonObject->SetStringField("Description", "Client already initialized.");
		DelegateConnection.ExecuteIfBound(false, TempCode);
		return;
	}

	AsyncTask(ENamedThreads::AnyNormalThreadNormalTask, [this, DelegateConnection, TempCode, In_Username, In_Pass, In_ClientId, In_Address, KeepAliveInterval, In_Version, bUseSSL, bUseWebSocket]()
		{
			int RetVal = -1;
			MQTTClient TempClient = nullptr;

			if (In_Version == EMQTTVERSION::V_5)
			{
				MQTTClient_createOptions createOpts = MQTTClient_createOptions_initializer;
				createOpts.MQTTVersion = MQTTVERSION_5;
				RetVal = MQTTClient_createWithOptions(&TempClient, TCHAR_TO_UTF8(*In_Address), TCHAR_TO_UTF8(*In_ClientId), MQTTCLIENT_PERSISTENCE_NONE, NULL, &createOpts);
			}

			else
			{
				RetVal = MQTTClient_create(&TempClient, TCHAR_TO_UTF8(*In_Address), TCHAR_TO_UTF8(*In_ClientId), MQTTCLIENT_PERSISTENCE_NONE, NULL);
			}
			
			if (RetVal != MQTTCLIENT_SUCCESS)
			{
				TempCode.JsonObject->SetStringField("Description", "There was a problem while creating client. Code : " + (FString)FString::FromInt(RetVal));
				MQTTClient_destroy(&TempClient);

				AsyncTask(ENamedThreads::GameThread, [DelegateConnection, TempCode]()
					{
						DelegateConnection.ExecuteIfBound(false, TempCode);
					}
				);

				return;
			}

			RetVal = MQTTClient_setCallbacks(TempClient, this, AMQTT_Manager_Paho_Sync::ConnectionLost, AMQTT_Manager_Paho_Sync::MessageArrived, AMQTT_Manager_Paho_Sync::MessageDelivered);

			if (RetVal != MQTTCLIENT_SUCCESS)
			{
				TempCode.JsonObject->SetStringField("Description", "There was a problem while setting callbacks. Code : " + (FString)FString::FromInt(RetVal));
				MQTTClient_destroy(&TempClient);

				AsyncTask(ENamedThreads::GameThread, [DelegateConnection, TempCode]()
					{
						DelegateConnection.ExecuteIfBound(false, TempCode);
					}
				);

				return;
			}

			if (In_Version == EMQTTVERSION::V_5)
			{
				if (bUseWebSocket)
				{
					this->Connection_Options = MQTTClient_connectOptions_initializer5_ws;
				}

				else
				{
					this->Connection_Options = MQTTClient_connectOptions_initializer5;
				}

				this->Connection_Options.cleanstart = 1;
			}

			else
			{
				if (bUseWebSocket)
				{
					this->Connection_Options = MQTTClient_connectOptions_initializer_ws;
				}

				else
				{
					this->Connection_Options = MQTTClient_connectOptions_initializer;
				}

				this->Connection_Options.cleansession = 1;
			}

			this->Connection_Options.keepAliveInterval = KeepAliveInterval;
			this->Connection_Options.username = TCHAR_TO_UTF8(*In_Username);
			this->Connection_Options.password = TCHAR_TO_UTF8(*In_Pass);
			this->Connection_Options.MQTTVersion = (int32)In_Version;

			if (bUseSSL)
			{
				this->SSL_Options = MQTTClient_SSLOptions_initializer;
				this->SSL_Options.enableServerCertAuth = 0;
				this->SSL_Options.verify = 1;
				this->SSL_Options.CApath = NULL;
				this->SSL_Options.keyStore = NULL;
				this->SSL_Options.trustStore = NULL;
				this->SSL_Options.privateKey = NULL;
				this->SSL_Options.privateKeyPassword = NULL;
				this->SSL_Options.enabledCipherSuites = NULL;

				this->Connection_Options.ssl = &this->SSL_Options;
			}

			if (In_Version == EMQTTVERSION::V_5)
			{
				MQTTProperties PropertiesConnection = MQTTProperties_initializer;
				MQTTProperties PropertiesWill = MQTTProperties_initializer;
				const MQTTResponse Response = MQTTClient_connect5(TempClient, &this->Connection_Options, &PropertiesConnection, &PropertiesWill);
				RetVal = Response.reasonCode;
			}

			else
			{
				RetVal = MQTTClient_connect(TempClient, &this->Connection_Options);
			}

			if (RetVal != MQTTCLIENT_SUCCESS)
			{
				TempCode.JsonObject->SetStringField("Description", "There was a problem while making connection. Code : " + (FString)FString::FromInt(RetVal));
				MQTTClient_destroy(&TempClient);

				AsyncTask(ENamedThreads::GameThread, [DelegateConnection, TempCode]()
					{
						DelegateConnection.ExecuteIfBound(false, TempCode);
					}
				);		

				return;
			}

			AsyncTask(ENamedThreads::GameThread, [this, DelegateConnection, TempCode, TempClient, In_ClientId]()
				{
					this->Client = TempClient;
					this->ClientId = In_ClientId;
					TempCode.JsonObject->SetStringField("Description", "Connection successful.");

					DelegateConnection.ExecuteIfBound(true, TempCode);
				}
			);
		}
	);
}

void AMQTT_Manager_Paho_Sync::MQTT_Sync_Destroy()
{
	if (!this->Client)
	{
		return;
	}	

	if (MQTTClient_isConnected(this->Client))
	{
		if (this->Connection_Options.MQTTVersion == MQTTVERSION_5)
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

bool AMQTT_Manager_Paho_Sync::MQTT_Sync_Publish(FJsonObjectWrapper& Out_Code, FString In_Topic, FString In_Payload, EMQTTQOS In_QoS, int32 In_Retained)
{
	Out_Code.JsonObject->SetStringField("ClassName", "AMQTT_Manager_Paho_Sync");
	Out_Code.JsonObject->SetStringField("FunctionName", "MQTT_Publish");
	Out_Code.JsonObject->SetStringField("AdditionalInfo", "");

	if (!this->Client)
	{
		Out_Code.JsonObject->SetStringField("Description", "Client is not valid.");
		return false;
	}

	if (!MQTTClient_isConnected(this->Client))
	{
		Out_Code.JsonObject->SetStringField("Description", "Client is not connected.");
		return false;
	}

	int RetVal = -1; 
	MQTTClient_deliveryToken DeliveryToken;
	const int32 QoS = FMath::Clamp((int32)In_QoS, 0, 2);

	if (this->Connection_Options.MQTTVersion == MQTTVERSION_5)
	{
		MQTTProperties Properties_Publish = MQTTProperties_initializer;
		const MQTTResponse Response = MQTTClient_publish5(this->Client, TCHAR_TO_UTF8(*In_Topic), In_Payload.Len(), TCHAR_TO_UTF8(*In_Payload), QoS, In_Retained, &Properties_Publish, &DeliveryToken);
		RetVal = Response.reasonCode;
	}

	else
	{
		RetVal = MQTTClient_publish(this->Client, TCHAR_TO_UTF8(*In_Topic), In_Payload.Len(), TCHAR_TO_UTF8(*In_Payload), QoS, In_Retained, &DeliveryToken);
	}

	const FString DescSring = RetVal == MQTTCLIENT_SUCCESS ? "Payload successfully published." : "There was a problem while publishing payload with these configurations. : " + FString::FromInt(RetVal);
	Out_Code.JsonObject->SetStringField("Description", DescSring);

	return RetVal == MQTTCLIENT_SUCCESS ? true : false;
}

bool AMQTT_Manager_Paho_Sync::MQTT_Sync_Subscribe(FJsonObjectWrapper& Out_Code, FString In_Topic, EMQTTQOS In_QoS)
{
	Out_Code.JsonObject->SetStringField("ClassName", "AMQTT_Manager_Paho_Sync");
	Out_Code.JsonObject->SetStringField("FunctionName", "MQTT_Publish");
	Out_Code.JsonObject->SetStringField("AdditionalInfo", "");

	if (!this->Client)
	{
		Out_Code.JsonObject->SetStringField("Description", "Client is not valid.");
		return false;
	}

	if (!MQTTClient_isConnected(this->Client))
	{
		Out_Code.JsonObject->SetStringField("Description", "Client is not connected.");
		return false;
	}

	int RetVal = -1;
	const int32 QoS = FMath::Clamp((int32)In_QoS, 0, 2);
	
	if (this->Connection_Options.MQTTVersion == MQTTVERSION_5)
	{
		const MQTTResponse Response = MQTTClient_subscribe5(this->Client, TCHAR_TO_UTF8(*In_Topic), QoS, NULL, NULL);
		RetVal = Response.reasonCode;
	}

	else
	{
		RetVal = MQTTClient_subscribe(this->Client, TCHAR_TO_UTF8(*In_Topic), QoS);
	}

	const FString DescSring = RetVal == MQTTCLIENT_SUCCESS ? "Topic successfully subscribed." : "There was a problem while subscribing topic with these configurations. : " + FString::FromInt(RetVal);
	Out_Code.JsonObject->SetStringField("Description", DescSring);

	return RetVal == MQTTCLIENT_SUCCESS ? true : false;
}
