// Reads and writes the preset list to $profile:LoadoutEditorPresets.json.
class SCR_LoadoutPresetStorage
{
	static const string STORAGE_PATH = "$profile:LoadoutEditorPresets.json";
	static const int    MAX_PRESETS  = 20;

	//------------------------------------------------------------------------------------------------
	static array<ref SCR_LoadoutPreset> LoadAll()
	{
		array<ref SCR_LoadoutPreset> result = new array<ref SCR_LoadoutPreset>();

		if (!FileIO.FileExists(STORAGE_PATH))
			return result;

		FileHandle file = FileIO.OpenFile(STORAGE_PATH, FileMode.READ);
		if (!file)
			return result;

		// All preset data is written as a single line
		string content;
		file.ReadLine(content);
		file.Close();

		if (content.IsEmpty())
			return result;

		// Content is a JSON array: [ {...}, {...}, ... ]
		content = content.Trim();
		if (content.Length() < 2)
			return result;

		// Strip surrounding [ ]
		content = content.Substring(1, content.Length() - 2);

		array<string> rawPresets = new array<string>();
		SplitPresets(content, rawPresets);

		foreach (string raw : rawPresets)
		{
			raw = raw.Trim();
			if (raw.IsEmpty())
				continue;

			if (!raw.EndsWith("}"))
				raw += "}";

			SCR_LoadoutPreset preset = SCR_LoadoutPreset.Deserialize(raw);
			if (preset && !preset.name.IsEmpty())
				result.Insert(preset);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	static bool SaveAll(array<ref SCR_LoadoutPreset> presets)
	{
		if (!presets)
			return false;

		string content = "[";
		for (int i = 0; i < presets.Count(); i++)
		{
			if (i > 0)
				content += ",";
			content += presets[i].Serialize();
		}
		content += "]";

		FileHandle file = FileIO.OpenFile(STORAGE_PATH, FileMode.WRITE);
		if (!file)
		{
			Print("[LoadoutEditor] SaveAll — cannot open file for writing: " + STORAGE_PATH, LogLevel.WARNING);
			return false;
		}

		file.WriteLine(content);
		file.Close();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static bool SavePreset(SCR_LoadoutPreset preset)
	{
		if (!preset || preset.name.IsEmpty())
			return false;

		array<ref SCR_LoadoutPreset> all = LoadAll();

		for (int i = 0; i < all.Count(); i++)
		{
			if (all[i].name == preset.name)
			{
				all[i] = preset;
				return SaveAll(all);
			}
		}

		if (all.Count() >= MAX_PRESETS)
		{
			Print("[LoadoutEditor] Max presets reached (" + MAX_PRESETS + ").", LogLevel.WARNING);
			return false;
		}

		all.Insert(preset);
		return SaveAll(all);
	}

	//------------------------------------------------------------------------------------------------
	static bool DeletePreset(string name)
	{
		array<ref SCR_LoadoutPreset> all = LoadAll();
		for (int i = 0; i < all.Count(); i++)
		{
			if (all[i].name == name)
			{
				all.Remove(i);
				return SaveAll(all);
			}
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	static SCR_LoadoutPreset FindPreset(string name)
	{
		array<ref SCR_LoadoutPreset> all = LoadAll();
		foreach (SCR_LoadoutPreset p : all)
		{
			if (p.name == name)
				return p;
		}
		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected static void SplitPresets(string body, notnull array<string> outParts)
	{
		int depth = 0;
		int start = 0;
		int len   = body.Length();

		for (int i = 0; i < len; i++)
		{
			string ch = body.Get(i);
			if (ch == "{")
			{
				if (depth == 0) start = i;
				depth++;
			}
			else if (ch == "}")
			{
				depth--;
				if (depth == 0)
					outParts.Insert(body.Substring(start, i - start + 1));
			}
		}
	}
