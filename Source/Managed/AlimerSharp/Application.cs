// Copyright (c) Amer Koleci and contributors.
// Licensed under the Apache License, Version 2.0.

using System;
using System.Runtime.InteropServices;
using static Alimer.AlimerApi;

namespace Alimer
{
	[Preserve(AllMembers = true)]
	public class Application : IDisposable
	{
		private static Application _current;
		// references needed to prevent GC from collecting callbacks passed to native code
		private static ActionIntPtr _setupCallback;
		private static ActionIntPtr _startCallback;
		private static ActionIntPtr _stopCallback;

		private readonly IntPtr _handle;
		public static Application Current
		{
			get
			{
				if (_current == null)
					throw new InvalidOperationException("The application has not been created yet!");
				return _current;
			}
			private set { _current = value; }
		}

		[Preserve]
		protected Application()
		{
			// Keep references to callbacks (supposed to be passed to native code) as long as the App is alive
			_setupCallback = ProxySetup;
			_startCallback = ProxyStart;
			_stopCallback = ProxyStop;

			_handle = Application_new(_setupCallback, _startCallback, _stopCallback);
			_current = this;
		}

		public void Dispose()
		{

		}

		public int Run() => Application_Run(_handle);
		public void RunFrame() => Application_RunFrame(_handle);
		public void Exit() => Application_Exit(_handle);

		protected virtual void Setup() { }

		protected virtual void Initialize()
		{
		}

		[MonoPInvokeCallback(typeof(ActionIntPtr))]
		private static void ProxySetup(IntPtr handle)
		{
			Runtime.Setup();
			Current = GetApp(handle);
			Current.Setup();
		}

		[MonoPInvokeCallback(typeof(ActionIntPtr))]
		private static void ProxyStart(IntPtr handle)
		{
			Runtime.Start();
			Current = GetApp(handle);
			//Current.SubscribeToAppEvents();
			Current.Initialize();
		}

		[MonoPInvokeCallback(typeof(ActionIntPtr))]
		private static void ProxyStop(IntPtr handle)
		{
		}

		private static Application GetApp(IntPtr handle)
		{
			return _current;
		}
	}
}