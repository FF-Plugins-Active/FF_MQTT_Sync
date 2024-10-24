#include "MQTT_Manager_Sync.h"

bool AMQTT_Manager_Paho_Sync::SetSSLParams(FString In_Protocol, FPahoSslOptions In_Options)
{
	if (In_Protocol == "wss" || In_Protocol == "mqtts" || In_Protocol == "ssl" || In_Protocol == "WSS" || In_Protocol == "MQTTS" || In_Protocol == "SSL")
	{
		this->SSL_Options = MQTTClient_SSLOptions_initializer;
		this->SSL_Options.enableServerCertAuth = 0;
		this->SSL_Options.verify = 1;

		if (!In_Options.CAPath.IsEmpty() && FPaths::FileExists(In_Options.CAPath))
		{
			this->SSL_Options.CApath = TCHAR_TO_UTF8(*In_Options.CAPath);
		}

		if (!In_Options.Path_KeyStore.IsEmpty() && FPaths::FileExists(In_Options.Path_KeyStore))
		{
			this->SSL_Options.keyStore = TCHAR_TO_UTF8(*In_Options.Path_KeyStore);
		}

		if (!In_Options.Path_TrustStore.IsEmpty() && FPaths::FileExists(In_Options.Path_TrustStore))
		{
			this->SSL_Options.trustStore = TCHAR_TO_UTF8(*In_Options.Path_TrustStore);
		}

		if (!In_Options.Path_PrivateKey.IsEmpty() && FPaths::FileExists(In_Options.Path_PrivateKey))
		{
			this->SSL_Options.privateKey = TCHAR_TO_UTF8(*In_Options.Path_PrivateKey);
		}

		if (!In_Options.PrivateKeyPass.IsEmpty())
		{
			this->SSL_Options.privateKeyPassword = TCHAR_TO_UTF8(*In_Options.PrivateKeyPass);
		}

		if (!In_Options.CipherSuites.IsEmpty())
		{
			this->SSL_Options.enabledCipherSuites = TCHAR_TO_UTF8(*In_Options.CipherSuites);
		}

		return true;
	}

	else
	{
		return false;
	}
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