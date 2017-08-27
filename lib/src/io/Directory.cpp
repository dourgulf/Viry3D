#include "Directory.h"

#if VR_WINDOWS
#include <io.h>
#include <Windows.h>
#else
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

namespace Viry3D
{
#if VR_WINDOWS
	static Vector<String> get_files(String path, bool recursive, bool* exist, bool get_dirs_only = false)
	{
		Vector<String> files;
		Vector<String> dirs;

		String find_file = path + "/*.*";
		_finddata_t find_data;
		intptr_t find_handle = _findfirst(find_file.CString(), &find_data);
		intptr_t find = find_handle;

		if(exist != NULL)
		{
			if(find_handle != -1 && (find_data.attrib & _A_SUBDIR) != 0)
			{
				*exist = true;
			}
			else
			{
				*exist = false;
			}

			if(find_handle != -1)
			{
				_findclose(find_handle);
			}
			return files;
		}

		while(find != -1)
		{
			String name(find_data.name);
			if(find_data.attrib & _A_SUBDIR)
			{
				if(name != "." &&
					name != "..")
				{
					dirs.Add(name);
				}
			}
			else if(find_data.attrib & _A_ARCH)
			{
				files.Add(name);
			}

			find = _findnext(find_handle, &find_data);
		}

		if(find_handle != -1)
		{
			_findclose(find_handle);
		}

		if(get_dirs_only)
		{
			return dirs;
		}

		if(recursive)
		{
			for(auto& i : dirs)
			{
				auto sub_dir = path + "/" + i;
				auto sub_files = get_files(sub_dir, true, NULL, false);

				for(auto& j : sub_files)
				{
					files.Add(i + "/" + j);
				}
			}
		}

		return files;
	}
#else
	static Vector<String> get_files(String path, bool recursive, bool* exist, bool get_dirs_only = false)
	{
		Vector<String> files;
		Vector<String> dirs;

		DIR *dir = opendir(path.CString());

		if(exist != NULL)
		{
			if(dir != NULL && (readdir(dir)->d_type & DT_DIR) != 0)
			{
				*exist = true;
			}
			else
			{
				*exist = false;
			}

			if(dir != NULL)
			{
				closedir(dir);
			}
			return files;
		}

		if(dir != NULL)
		{
			dirent* p;
			while((p = readdir(dir)) != NULL)
			{
				String name = p->d_name;
				if(p->d_type & DT_DIR)
				{
					if(name != "." &&
						name != "..")
					{
						dirs.Add(name);
					}
				}
				else if(p->d_type & DT_REG)
				{
					files.Add(name);
				}
			}

			closedir(dir);
		}

		if(get_dirs_only)
		{
			return dirs;
		}

		if(recursive)
		{
			for(auto& i : dirs)
			{
				auto sub_dir = path + "/" + i;
				auto sub_files = get_files(sub_dir, true, NULL);

				for(auto& j : sub_files)
				{
					files.Add(i + "/" + j);
				}
			}
		}

		return files;
	}
#endif

	bool Directory::Exist(String path)
	{
		bool exist;
		get_files(path, false, &exist);
		return exist;
	}

	Vector<String> Directory::GetDirectorys(String path)
	{
		auto dirs = get_files(path, false, NULL, true);

		for(auto& i : dirs)
		{
			i = path + "/" + i;
		}

		return dirs;
	}

	Vector<String> Directory::GetFiles(String path, bool recursive)
	{
		auto files = get_files(path, recursive, NULL);

		for(auto& i : files)
		{
			i = path + "/" + i;
		}

		return files;
	}

	void Directory::Create(String path)
	{
		auto splits = path.Split("/", true);
		String folder = splits[0];

		if(path.StartsWith("/"))
		{
			folder = "/" + folder;
		}

		for(int i = 1; i < splits.Size(); i++)
		{
			folder += "/" + splits[i];

#if VR_WINDOWS
			CreateDirectoryA(folder.CString(), NULL);
#elif VR_ANDROID || VR_IOS
			mkdir(folder.CString(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
		}
	}
}