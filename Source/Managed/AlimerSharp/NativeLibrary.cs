// Copyright (c) Amer Koleci and contributors.
// Licensed under the Apache License, Version 2.0.

using System;
using System.Runtime.InteropServices;

namespace Alimer
{
	internal static class NativeLibrary
	{
		private const int RTLD_LAZY = 0x0001;
		private const int RTLD_GLOBAL = 0x0100;

		public static readonly bool IsWindows;
		public static readonly bool IsOSX;

		static NativeLibrary()
		{
			IsWindows = RuntimeInformation.IsOSPlatform(OSPlatform.Windows);
			IsOSX = RuntimeInformation.IsOSPlatform(OSPlatform.OSX);
		}

		public static IntPtr LoadLibrary(string libname)
		{
			if (IsWindows)
			{
				return Windows.LoadLibraryW(libname);
			}

			if (IsOSX)
			{
				return OSX.dlopen(libname, RTLD_GLOBAL | RTLD_LAZY);
			}

			return Linux.dlopen(libname, RTLD_GLOBAL | RTLD_LAZY);
		}

		public static T LoadFunction<T>(IntPtr library, string function)
		{
			var ret = IntPtr.Zero;

			if (IsWindows)
			{
				ret = Windows.GetProcAddress(library, function);
			}
			else if (IsOSX)
			{
				ret = OSX.dlsym(library, function);
			}
			else
			{
				ret = Linux.dlsym(library, function);
			}

			if (ret == IntPtr.Zero)
			{
				return default(T);
			}

			return Marshal.GetDelegateForFunctionPointer<T>(ret);
		}

		private static class Windows
		{
			[DllImport("kernel32", CharSet = CharSet.Ansi, ExactSpelling = true, SetLastError = true)]
			public static extern IntPtr GetProcAddress(IntPtr hModule, string procName);

			[DllImport("kernel32", SetLastError = true, CharSet = CharSet.Unicode)]
			public static extern IntPtr LoadLibraryW(string lpszLib);
		}

#pragma warning disable IDE1006 // Naming Styles
		private static class Linux
		{
			[DllImport("libdl.so.2")]
			public static extern IntPtr dlopen(string path, int flags);

			[DllImport("libdl.so.2")]
			public static extern IntPtr dlsym(IntPtr handle, string symbol);
		}

		private static class OSX
		{
			[DllImport("/usr/lib/libSystem.dylib")]
			public static extern IntPtr dlopen(string path, int flags);

			[DllImport("/usr/lib/libSystem.dylib")]
			public static extern IntPtr dlsym(IntPtr handle, string symbol);
		}
#pragma warning restore IDE1006 // Naming Styles
	}
}
