// Copyright (c) Amer Koleci and contributors.
// Licensed under the Apache License, Version 2.0.

namespace Alimer.Studio
{
	internal static class Program
	{
		public static void Main(string[] args)
		{
			using(var app = new StudioApplication())
			{
				app.Run();
			}
		}
	}
}