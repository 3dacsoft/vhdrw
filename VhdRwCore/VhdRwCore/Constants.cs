using System;

namespace VhdRwCore
{
	static class Constants
	{
		public const int MinArguments = 4;
		public const int MaxArguments = 4;
		public const int SectorSize = 512;
		public const int FooterSize = 512;
		public const int HeaderSize = 1024;
		public const int LegacyFooterSize = 511;
		public const int BATEntrySize = 4;
		public const int EmptyBATEntry = -1;
		public const string FooterSignature = "conectix";
		public static DateTime TimeStampBase = new DateTime(2000, 1, 1, 0, 0, 0, DateTimeKind.Utc);
	}
	public enum DiskOperations
	{
		Read = 'r',
		Write = 'w'
	}

	static class Utilities
	{
		public static ushort Swap16(ushort word) => (ushort)(word >> 8 | word << 8);
		public static uint Swap32(uint dword) =>
			((dword >> 24) & 0xff) | ((dword << 8) & 0xff0000) |
			((dword >> 8) & 0xff00) | ((dword << 24) & 0xff000000);
		public static ulong Swap64(ulong qword) =>
			((qword >> 56) & 0xff) | ((qword >> 40) & 0xff00) |
			((qword >> 24) & 0xff0000) | ((qword >> 8) & 0xff000000) |
			((qword << 8) & 0xff00000000) | ((qword << 24) & 0xff0000000000) |
			((qword << 40) & 0xff000000000000) | ((qword << 56) & 0xff00000000000000);
	}
}

