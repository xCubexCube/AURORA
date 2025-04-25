#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "IWebSocket.h"
#include "AuroraGameInstance.generated.h"

// ?? Mover el delegado aquí, fuera de la clase, pero antes de la declaración de la clase
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnVectorsReceivedSignature, FVector, Accel, FVector, RotationRate, FVector2D, Touch);

/**
 * Clase GameInstance para manejar la conexión WebSocket
 */
UCLASS()
class AURORA_API UAuroraGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void Shutdown() override;

	UFUNCTION(BlueprintCallable, Category = "WebSocket")
	void SendMessageToServer(const FString& Message);

	UFUNCTION(BlueprintCallable, Category = "WebSocket")
	void SendSensorData(float TouchX, float TouchY, FVector Accel, FVector RotationRate);

	// ? Evento accesible desde Blueprints
	UPROPERTY(BlueprintAssignable, Category = "WebSocket")
	FOnVectorsReceivedSignature OnVectorsReceived;

private:
	TSharedPtr<IWebSocket> WebSocket;
};
