// Hooks into the player-side inventory manager to detect arsenal use.
// Automatically opens / closes the loadout editor UI.
modded class SCR_InventoryStorageManagerComponent
{
	protected ref SCR_LoadoutEditorUI m_LoadoutEditorUI;

	// Scratch field used by the entity sphere query callback.
	protected SCR_ArsenalInventoryStorageManagerComponent m_FoundArsenal;

	//------------------------------------------------------------------------------------------------
	override void OpenInventory()
	{
		super.OpenInventory();

		if (!IsLocalPlayerOwner())
			return;

		SCR_ArsenalInventoryStorageManagerComponent arsenal = FindNearbyArsenal();
		if (!arsenal)
			return;

		m_LoadoutEditorUI = new SCR_LoadoutEditorUI();
		m_LoadoutEditorUI.Open(arsenal, GetOwner());
	}

	//------------------------------------------------------------------------------------------------
	override void OnInventoryMenuClosed()
	{
		super.OnInventoryMenuClosed();

		if (m_LoadoutEditorUI)
		{
			m_LoadoutEditorUI.Close();
			m_LoadoutEditorUI = null;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsLocalPlayerOwner()
	{
		IEntity owner = GetOwner();
		IEntity local = SCR_PlayerController.GetLocalControlledEntity();
		return owner && local && owner == local;
	}

	//------------------------------------------------------------------------------------------------
	// Sphere query callback — return false to stop the query once the arsenal is found.
	protected bool OnArsenalQuery(IEntity entity)
	{
		if (!entity)
			return true;

		m_FoundArsenal = SCR_ArsenalInventoryStorageManagerComponent.Cast(
			entity.FindComponent(SCR_ArsenalInventoryStorageManagerComponent));

		return (m_FoundArsenal == null);
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_ArsenalInventoryStorageManagerComponent FindNearbyArsenal()
	{
		IEntity owner = GetOwner();
		if (!owner)
			return null;

		m_FoundArsenal = null;
		GetGame().GetWorld().QueryEntitiesBySphere(owner.GetOrigin(), 4.0, OnArsenalQuery);
		return m_FoundArsenal;
	}
}
