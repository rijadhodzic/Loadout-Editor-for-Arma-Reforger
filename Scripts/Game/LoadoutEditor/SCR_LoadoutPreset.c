// Serializable preset data. All ResourceName fields stored as plain string
// so JsonSerializer can round-trip them without type mismatch.
class SCR_LoadoutPreset
{
	string name;

	// Weapons
	string primaryWeapon;
	string secondaryWeapon;
	string launcher;

	// Clothing
	string uniform;
	string vest;
	string backpack;
	string headgear;
	string facewear;

	// Attachment pairs — parallel arrays (slot name, item path)
	ref array<string> primaryAttachmentSlots;
	ref array<string> primaryAttachmentItems;
	ref array<string> secondaryAttachmentSlots;
	ref array<string> secondaryAttachmentItems;

	// Misc items
	ref array<string> items;

	//------------------------------------------------------------------------------------------------
	void SCR_LoadoutPreset()
	{
		primaryAttachmentSlots   = new array<string>();
		primaryAttachmentItems   = new array<string>();
		secondaryAttachmentSlots = new array<string>();
		secondaryAttachmentItems = new array<string>();
		items                    = new array<string>();
	}

	//------------------------------------------------------------------------------------------------
	static SCR_LoadoutPreset CreateCopy(SCR_LoadoutPreset src)
	{
		SCR_LoadoutPreset p          = new SCR_LoadoutPreset();
		p.name                       = src.name;
		p.primaryWeapon              = src.primaryWeapon;
		p.secondaryWeapon            = src.secondaryWeapon;
		p.launcher                   = src.launcher;
		p.uniform                    = src.uniform;
		p.vest                       = src.vest;
		p.backpack                   = src.backpack;
		p.headgear                   = src.headgear;
		p.facewear                   = src.facewear;
		p.primaryAttachmentSlots.Copy(src.primaryAttachmentSlots);
		p.primaryAttachmentItems.Copy(src.primaryAttachmentItems);
		p.secondaryAttachmentSlots.Copy(src.secondaryAttachmentSlots);
		p.secondaryAttachmentItems.Copy(src.secondaryAttachmentItems);
		p.items.Copy(src.items);
		return p;
	}

	//------------------------------------------------------------------------------------------------
	// Flat JSON serialization — no external library required.
	string Serialize()
	{
		string s = "{";
		s += "\"name\":\"" + name + "\",";
		s += "\"primaryWeapon\":\"" + primaryWeapon + "\",";
		s += "\"secondaryWeapon\":\"" + secondaryWeapon + "\",";
		s += "\"launcher\":\"" + launcher + "\",";
		s += "\"uniform\":\"" + uniform + "\",";
		s += "\"vest\":\"" + vest + "\",";
		s += "\"backpack\":\"" + backpack + "\",";
		s += "\"headgear\":\"" + headgear + "\",";
		s += "\"facewear\":\"" + facewear + "\",";
		s += "\"priSlots\":"  + SerializeStringArray(primaryAttachmentSlots)   + ",";
		s += "\"priItems\":"  + SerializeStringArray(primaryAttachmentItems)   + ",";
		s += "\"secSlots\":"  + SerializeStringArray(secondaryAttachmentSlots) + ",";
		s += "\"secItems\":"  + SerializeStringArray(secondaryAttachmentItems) + ",";
		s += "\"items\":"     + SerializeStringArray(items);
		s += "}";
		return s;
	}

	//------------------------------------------------------------------------------------------------
	static SCR_LoadoutPreset Deserialize(string raw)
	{
		SCR_LoadoutPreset p  = new SCR_LoadoutPreset();
		p.name               = ExtractValue(raw, "name");
		p.primaryWeapon      = ExtractValue(raw, "primaryWeapon");
		p.secondaryWeapon    = ExtractValue(raw, "secondaryWeapon");
		p.launcher           = ExtractValue(raw, "launcher");
		p.uniform            = ExtractValue(raw, "uniform");
		p.vest               = ExtractValue(raw, "vest");
		p.backpack           = ExtractValue(raw, "backpack");
		p.headgear           = ExtractValue(raw, "headgear");
		p.facewear           = ExtractValue(raw, "facewear");
		ExtractArray(raw, "priSlots",  p.primaryAttachmentSlots);
		ExtractArray(raw, "priItems",  p.primaryAttachmentItems);
		ExtractArray(raw, "secSlots",  p.secondaryAttachmentSlots);
		ExtractArray(raw, "secItems",  p.secondaryAttachmentItems);
		ExtractArray(raw, "items",     p.items);
		return p;
	}

	//------------------------------------------------------------------------------------------------
	protected static string SerializeStringArray(array<string> arr)
	{
		string s = "[";
		for (int i = 0; i < arr.Count(); i++)
		{
			if (i > 0) s += ",";
			s += "\"" + arr[i] + "\"";
		}
		s += "]";
		return s;
	}

	//------------------------------------------------------------------------------------------------
	// IndexOf in EnforceScript only accepts one argument.
	// Simulate start-offset search by working on a substring.
	protected static string ExtractValue(string json, string key)
	{
		string search = "\"" + key + "\":\"";
		int idx = json.IndexOf(search);
		if (idx < 0)
			return string.Empty;

		// Advance past the key+colon+opening-quote
		int startPos = idx + search.Length();
		string after = json.Substring(startPos, json.Length() - startPos);
		int endPos = after.IndexOf("\"");
		if (endPos < 0)
			return string.Empty;

		return after.Substring(0, endPos);
	}

	//------------------------------------------------------------------------------------------------
	protected static void ExtractArray(string json, string key, notnull array<string> outArr)
	{
		string search = "\"" + key + "\":[";
		int idx = json.IndexOf(search);
		if (idx < 0)
			return;

		int startPos2 = idx + search.Length();
		string after = json.Substring(startPos2, json.Length() - startPos2);
		int endPos = after.IndexOf("]");
		if (endPos < 0)
			return;

		string inner = after.Substring(0, endPos);
		inner.Replace("\"", "");
		if (inner.IsEmpty())
			return;

		inner.Split(",", outArr, true);
	}
}
