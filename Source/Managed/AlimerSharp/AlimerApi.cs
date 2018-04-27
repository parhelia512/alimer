// Copyright (c) Amer Koleci and contributors.
// Licensed under the Apache License, Version 2.0.

using System;
using System.IO;
using System.Runtime.InteropServices;

namespace Alimer
{
	partial class AlimerApi
	{
		private const string Library = "libAlimerSharp";

		[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
		public delegate void ActionIntPtr(IntPtr value);

		[DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
		public extern static IntPtr Application_new(ActionIntPtr setup, ActionIntPtr start, ActionIntPtr stop);

		[DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
		public extern static int Application_Run(IntPtr handle);

		[DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
		public extern static void Application_RunFrame(IntPtr handle);

		[DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
		public extern static void Application_Exit(IntPtr handle);
	}
}
