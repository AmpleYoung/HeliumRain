#pragma once

#include "FlareQuestCatalogEntry.h"
#include "FlareQuestCatalog.generated.h"


UCLASS()
class UFlareQuestCatalog : public UDataAsset
{
	GENERATED_UCLASS_BODY()

public:

	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/

	/** Sectors data */
	UPROPERTY(EditAnywhere, Category = Content)
	TArray<UFlareQuestCatalogEntry*> Quests;

};
