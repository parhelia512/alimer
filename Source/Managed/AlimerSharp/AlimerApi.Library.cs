// Copyright (c) Amer Koleci and contributors.
// Licensed under the Apache License, Version 2.0.

using System;
using System.IO;
using System.Runtime.InteropServices;

namespace Alimer
{
	internal unsafe static partial class AlimerApi
	{
#if TODO
		public static readonly IntPtr _library;

		static AlimerApi()
		{
			var handle = IntPtr.Zero;

			// Load bundled library.
			var assemblyLocation = AppContext.BaseDirectory;
			if (NativeLibrary.IsWindows)
			{
				if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
				{
					handle = NativeLibrary.LoadLibrary(Path.Combine(assemblyLocation, "libAlimerSharp.dll"));
					if (handle != IntPtr.Zero)
					{
						_library = handle;
						return;
					}
				}
				else if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
				{
					var libs = new[]
					{
						"libAlimerSharp.so",
						"libAlimerSharp.so.0",
						"libAlimerSharp.so.1"
					};

					foreach (var libName in libs)
					{
						handle = NativeLibrary.LoadLibrary(Path.Combine(assemblyLocation, libName));
						if (handle != IntPtr.Zero)
						{
							_library = handle;
							return;
						}
					}
				}
				else if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
				{
					handle = NativeLibrary.LoadLibrary(Path.Combine(assemblyLocation, "libAlimerSharp.dylib"));
					if (handle != IntPtr.Zero)
					{
						_library = handle;
						return;
					}
				}
			}

			// Load system library
			if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
			{
				handle = NativeLibrary.LoadLibrary("libAlimerSharp.dll");
			}
			else if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
			{
				handle = NativeLibrary.LoadLibrary("libAlimerSharp.so.0");
			}
			else if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
			{
				handle = NativeLibrary.LoadLibrary("libAlimerSharp.dylib");
			}

			// Throw exception if not loaded so far.
			if (handle == IntPtr.Zero)
			{
				throw new Exception("Failed to load AlimerSharp native library.");
			}

			_library = handle;
		}

		public static T LoadFunction<T>(string function)
		{
			return NativeLibrary.LoadFunction<T>(_library, function);
		} 
#endif
	}
}
