//Fill out your copyright notice in the Description page of Project Settings.

#include "MQTT_Manager_Paho_Sync.h"

// Sets default values.
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

// Called every frame.
void AMQTT_Manager_Paho_Sync::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

#pragma region CALLBACKS
void AMQTT_Manager_Paho_Sync::MessageDelivered(void* CallbackContext, MQTTClient_deliveryToken In_DeliveryToken)
{
	AMQTT_Manager_Paho_Sync* Owner = Cast<AMQTT_Manager_Paho_Sync>((AMQTT_Manager_Paho_Sync*)CallbackContext);

	if (!Owner)
	{
		return;
	}

	Owner->Delegate_Delivered.Broadcast(FString::FromInt(In_DeliveryToken));
}

int AMQTT_Manager_Paho_Sync::MessageArrived(void* CallbackContext, char* TopicName, int TopicLenght, MQTTClient_message* Message)
{
	AMQTT_Manager_Paho_Sync* Owner = Cast<AMQTT_Manager_Paho_Sync>((AMQTT_Manager_Paho_Sync*)CallbackContext);

	if (!Owner)
	{
		return 0;
	}

	FPahoArrived_Sync StrArrived;
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
bool AMQTT_Manager_Paho_Sync::SetSSLParams(FString In_Protocol, FPahoClientParams_Sync In_Params)
{
	if (In_Params.Address.IsEmpty())
	{
		return false;
	}

	if (In_Protocol.IsEmpty())
	{
		return false;
	}

	if (In_Protocol == "wss" || In_Protocol == "mqtts" || In_Protocol == "ssl" || In_Protocol == "WSS" || In_Protocol == "MQTTS" || In_Protocol == "SSL")
	{
		this->SSL_Options = MQTTClient_SSLOptions_initializer;
		this->SSL_Options.enableServerCertAuth = 0;
		this->SSL_Options.verify = 1;

		if (!In_Params.CAPath.IsEmpty() && FPaths::FileExists(In_Params.CAPath))
		{
			this->SSL_Options.CApath = TCHAR_TO_UTF8(*In_Params.CAPath);
		}

		else
		{
			this->SSL_Options.CApath = NULL;
		}

		if (!In_Params.Path_KeyStore.IsEmpty() && FPaths::FileExists(In_Params.Path_KeyStore))
		{
			this->SSL_Options.keyStore = TCHAR_TO_UTF8(*In_Params.Path_KeyStore);
		}

		else
		{
			this->SSL_Options.keyStore = NULL;
		}

		if (!In_Params.Path_TrustStore.IsEmpty() && FPaths::FileExists(In_Params.Path_TrustStore))
		{
			this->SSL_Options.trustStore = TCHAR_TO_UTF8(*In_Params.Path_TrustStore);
		}

		else
		{
			this->SSL_Options.trustStore = NULL;
		}

		if (!In_Params.Path_PrivateKey.IsEmpty() && FPaths::FileExists(In_Params.Path_PrivateKey))
		{

			this->SSL_Options.privateKey = TCHAR_TO_UTF8(*In_Params.Path_PrivateKey);
		}

		else
		{
			this->SSL_Options.privateKey = NULL;
		}

		this->SSL_Options.privateKeyPassword = In_Params.PrivateKeyPass.IsEmpty() ? NULL : TCHAR_TO_UTF8(*In_Params.PrivateKeyPass);
		this->SSL_Options.enabledCipherSuites = In_Params.CipherSuites.IsEmpty() ? NULL : TCHAR_TO_UTF8(*In_Params.CipherSuites);

		return true;
	}

	else
	{
		return false;
	}
}
#pragma endregion CALLBACKS

FPahoClientParams_Sync AMQTT_Manager_Paho_Sync::GetClientParams()
{
	return this->Client_Params;
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

void AMQTT_Manager_Paho_Sync::MQTT_Sync_Init(FDelegate_Paho_Connection_Sync DelegateConnection, FPahoClientParams_Sync In_Params)
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

	if (!In_Params.IsParamsValid())
	{
		TempCode.JsonObject->SetStringField("Description", "Address and/or client id should not be empty.");
		DelegateConnection.ExecuteIfBound(false, TempCode);
		return;
	}

	AsyncTask(ENamedThreads::AnyNormalThreadNormalTask, [this, DelegateConnection, TempCode, In_Params]()
		{
			FString Protocol;
			TArray<FString> URL_Sections = UKismetStringLibrary::ParseIntoArray(In_Params.Address, "://");

			if (URL_Sections.Num() > 1)
			{
				Protocol = URL_Sections[0];
				this->SetSSLParams(Protocol, In_Params);
			}

			int RetVal = -1;
			MQTTClient TempClient = nullptr;

			if (In_Params.Version == EMQTTVERSION_Sync::V_5)
			{
				MQTTClient_createOptions Create_Options = MQTTClient_createOptions_initializer;
				Create_Options.MQTTVersion = MQTTVERSION_5;
				
				RetVal = MQTTClient_createWithOptions(&TempClient, TCHAR_TO_UTF8(*In_Params.Address), TCHAR_TO_UTF8(*In_Params.ClientId), MQTTCLIENT_PERSISTENCE_NONE, NULL, &Create_Options);

				if (Protocol == "wss" || Protocol == "ws")
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
				RetVal = MQTTClient_create(&TempClient, TCHAR_TO_UTF8(*In_Params.Address), TCHAR_TO_UTF8(*In_Params.ClientId), MQTTCLIENT_PERSISTENCE_NONE, NULL);

				if (Protocol == "wss" || Protocol == "ws")
				{
					this->Connection_Options = MQTTClient_connectOptions_initializer_ws;
				}

				else
				{
					this->Connection_Options = MQTTClient_connectOptions_initializer;
				}

				this->Connection_Options.cleansession = 1;
			}

			this->Connection_Options.keepAliveInterval = In_Params.KeepAliveInterval;
			this->Connection_Options.username = TCHAR_TO_UTF8(*In_Params.UserName);
			this->Connection_Options.password = TCHAR_TO_UTF8(*In_Params.Password);
			this->Connection_Options.MQTTVersion = (int32)In_Params.Version;
			this->Connection_Options.ssl = &this->SSL_Options;

			if (RetVal != MQTTCLIENT_SUCCESS)
			{
				TempCode.JsonObject->SetStringField("Description", "There was a problem while creating client.");
				TempCode.JsonObject->SetStringField("ErrorCode", FString::FromInt(RetVal));

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
				TempCode.JsonObject->SetStringField("Description", "There was a problem while setting callbacks.");
				TempCode.JsonObject->SetStringField("ErrorCode", FString::FromInt(RetVal));

				MQTTClient_destroy(&TempClient);

				AsyncTask(ENamedThreads::GameThread, [DelegateConnection, TempCode]()
					{
						DelegateConnection.ExecuteIfBound(false, TempCode);
					}
				);

				return;
			}

			if (In_Params.Version == EMQTTVERSION_Sync::V_5)
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
				TempCode.JsonObject->SetStringField("Description", "There was a problem while making connection.");
				TempCode.JsonObject->SetStringField("ErrorCode", FString::FromInt(RetVal));

				MQTTClient_destroy(&TempClient);

				AsyncTask(ENamedThreads::GameThread, [DelegateConnection, TempCode]()
					{
						DelegateConnection.ExecuteIfBound(false, TempCode);
					}
				);		

				return;
			}

			this->Client = TempClient;
			this->Client_Params = In_Params;
			TempCode.JsonObject->SetStringField("Description", "Connection successful.");

			AsyncTask(ENamedThreads::GameThread, [DelegateConnection, TempCode]()
				{
					DelegateConnection.ExecuteIfBound(true, TempCode);
				}
			);
		}
	);
}

bool AMQTT_Manager_Paho_Sync::MQTT_Sync_Publish(FJsonObjectWrapper& Out_Code, FString In_Topic, FString In_Payload, EMQTTQOS_Sync In_QoS, int32 In_Retained)
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

	if (this->Connection_Options.MQTTVersion == MQTTVERSION_5)
	{
		MQTTProperties Properties_Publish = MQTTProperties_initializer;
		const MQTTResponse Response = MQTTClient_publish5(this->Client, TCHAR_TO_UTF8(*In_Topic), In_Payload.Len(), TCHAR_TO_UTF8(*In_Payload), (int32)In_QoS, In_Retained, &Properties_Publish, &DeliveryToken);
		RetVal = Response.reasonCode;
	}

	else
	{
		RetVal = MQTTClient_publish(this->Client, TCHAR_TO_UTF8(*In_Topic), In_Payload.Len(), TCHAR_TO_UTF8(*In_Payload), (int32)In_QoS, In_Retained, &DeliveryToken);
	}

	const FString DescSring = RetVal == MQTTCLIENT_SUCCESS ? "Payload successfully published." : "There was a problem while publishing payload with these configurations.";
	Out_Code.JsonObject->SetStringField("Description", DescSring);
	Out_Code.JsonObject->SetStringField("ErrorCode", FString::FromInt(RetVal));
	
	return RetVal == MQTTCLIENT_SUCCESS ? true : false;
}

bool AMQTT_Manager_Paho_Sync::MQTT_Sync_Subscribe(FJsonObjectWrapper& Out_Code, FString In_Topic, EMQTTQOS_Sync In_QoS)
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
	
	if (this->Connection_Options.MQTTVersion == MQTTVERSION_5)
	{
		const MQTTResponse Response = MQTTClient_subscribe5(this->Client, TCHAR_TO_UTF8(*In_Topic), (int32)In_QoS, NULL, NULL);
		RetVal = Response.reasonCode;
	}

	else
	{
		RetVal = MQTTClient_subscribe(this->Client, TCHAR_TO_UTF8(*In_Topic), (int32)In_QoS);
	}

	const FString DescSring = RetVal == MQTTCLIENT_SUCCESS ? "Topic successfully subscribed." : "There was a problem while subscribing topic with these configurations.";
	Out_Code.JsonObject->SetStringField("Description", DescSring);
	Out_Code.JsonObject->SetStringField("ErrorCode", FString::FromInt(RetVal));
	
	return RetVal == MQTTCLIENT_SUCCESS ? true : false;
}

bool AMQTT_Manager_Paho_Sync::MQTT_Sync_Unsubscribe(FJsonObjectWrapper& Out_Code, FString In_Topic)
{
	Out_Code.JsonObject->SetStringField("ClassName", "AMQTT_Manager_Paho_Sync");
	Out_Code.JsonObject->SetStringField("FunctionName", "MQTT_Sync_Unsubscribe");
	Out_Code.JsonObject->SetStringField("AdditionalInfo", "");
	Out_Code.JsonObject->SetStringField("ErrorCode", "");

	if (!this->Client)
	{
		Out_Code.JsonObject->SetStringField("Description", "Client is not valid.");
		return false;
	}

	if (!MQTTClient_isConnected(this->Client))
	{
		Out_Code.JsonObject->SetStringField("Description", "Client is not connected.");
		Out_Code.JsonObject->SetStringField("AdditionalInfo", "Try to give some delay before using this or use it after \"Delegate OnConnect\"");
		return false;
	}

	const int RetVal = MQTTClient_unsubscribe(this->Client, TCHAR_TO_UTF8(*In_Topic));

	const FString Description = RetVal == MQTTCLIENT_SUCCESS ? "Target successfully unsubscribed." : "There was a problem while unsubscribing target.";
	Out_Code.JsonObject->SetStringField("Description", Description);
	Out_Code.JsonObject->SetStringField("ErrorCode", FString::FromInt(RetVal));

	return RetVal == MQTTCLIENT_SUCCESS ? true : false;
}
