#include "AuroraGameInstance.h"
#include "WebSocketsModule.h"
#include "Engine/Engine.h"
#include "Json.h"
#include "JsonUtilities.h"

void UAuroraGameInstance::Init()
{
	Super::Init();

	// Asegúrate de que el módulo WebSockets esté cargado
	if (!FModuleManager::Get().IsModuleLoaded("WebSockets"))
	{
		FModuleManager::Get().LoadModule("WebSockets");
	}

	if (FModuleManager::Get().IsModuleLoaded("WebSockets"))
	{
		// Dirección del servidor WebSocket (ajusta según tu IP y puerto)
		//WebSocket = FWebSocketsModule::Get().CreateWebSocket("ws://192.168.100.85:8080");
		WebSocket = FWebSocketsModule::Get().CreateWebSocket("ws://localhost:8080");

		WebSocket->OnConnected().AddLambda([]()
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, TEXT("WebSocket conectado correctamente"));
		});

		WebSocket->OnConnectionError().AddLambda([](const FString& Error)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString::Printf(TEXT("Error de conexión: %s"), *Error));
		});

		WebSocket->OnClosed().AddLambda([](int32 StatusCode, const FString& Reason, bool bWasClean)
		{
			const FColor Color = bWasClean ? FColor::Green : FColor::Red;
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, Color, FString::Printf(TEXT("Conexión cerrada: %s"), *Reason));
		});

		WebSocket->OnMessage().AddLambda([this](const FString& MessageString)
		{
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(MessageString);

			if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
			{
				// Leer Touch como Vector2D
				FVector2D Touch(
					JsonObject->GetNumberField(TEXT("TouchX")),
					JsonObject->GetNumberField(TEXT("TouchY"))
				);

				// Leer Accel como FVector
				TSharedPtr<FJsonObject> AccelJson = JsonObject->GetObjectField(TEXT("Accel"));
				FVector Accel(
					AccelJson->GetNumberField(TEXT("X")),
					AccelJson->GetNumberField(TEXT("Y")),
					AccelJson->GetNumberField(TEXT("Z"))
				);

				// Leer RotationRate como FVector
				TSharedPtr<FJsonObject> RotationJson = JsonObject->GetObjectField(TEXT("RotationRate"));
				FVector RotationRate(
					RotationJson->GetNumberField(TEXT("X")),
					RotationJson->GetNumberField(TEXT("Y")),
					RotationJson->GetNumberField(TEXT("Z"))
				);

				// Emitir evento para Blueprints
				OnVectorsReceived.Broadcast(Accel, RotationRate, Touch);
			}
		});

		WebSocket->Connect();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No se pudo cargar el módulo WebSockets"));
	}
}

void UAuroraGameInstance::Shutdown()
{
	if (WebSocket.IsValid() && WebSocket->IsConnected())
	{
		WebSocket->Close();
	}
	Super::Shutdown();
}

void UAuroraGameInstance::SendMessageToServer(const FString& Message)
{
	if (WebSocket.IsValid() && WebSocket->IsConnected())
	{
		WebSocket->Send(Message);
	}
}

void UAuroraGameInstance::SendSensorData(float TouchX, float TouchY, FVector Accel, FVector RotationRate)
{
	if (WebSocket && WebSocket->IsConnected())
	{
		TSharedPtr<FJsonObject> MessageJson = MakeShareable(new FJsonObject);
		MessageJson->SetNumberField(TEXT("TouchX"), TouchX);
		MessageJson->SetNumberField(TEXT("TouchY"), TouchY);

		TSharedPtr<FJsonObject> AccelJson = MakeShareable(new FJsonObject);
		AccelJson->SetNumberField(TEXT("X"), Accel.X);
		AccelJson->SetNumberField(TEXT("Y"), Accel.Y);
		AccelJson->SetNumberField(TEXT("Z"), Accel.Z);
		MessageJson->SetObjectField(TEXT("Accel"), AccelJson);

		TSharedPtr<FJsonObject> RotationJson = MakeShareable(new FJsonObject);
		RotationJson->SetNumberField(TEXT("X"), RotationRate.X);
		RotationJson->SetNumberField(TEXT("Y"), RotationRate.Y);
		RotationJson->SetNumberField(TEXT("Z"), RotationRate.Z);
		MessageJson->SetObjectField(TEXT("RotationRate"), RotationJson);

		FString MessageString;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&MessageString);
		FJsonSerializer::Serialize(MessageJson.ToSharedRef(), Writer);

		WebSocket->Send(MessageString);
	}
}
