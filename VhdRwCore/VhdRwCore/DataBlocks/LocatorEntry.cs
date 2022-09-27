using System;
using System.Text;

namespace VhdRwCore
{
	class LocatorEntry : DataBlock
	{
		public string PlatformCode => ExtractString(0, Encoding.ASCII, 4);
		public int DataSpace => ExtractNumber<int>(4);
		public int DataLength => ExtractNumber<int>(8);
		public long DataOffset => ExtractNumber<long>(16);

		public LocatorEntry(ReadOnlySpan<byte> data) : base(data.ToArray()) { }
	}

	class LocatorEntryCollection
	{
		private readonly LocatorEntry[] entries;
		public LocatorEntryCollection(Span<byte> block)
		{
			entries = new LocatorEntry[7];
			for (int i = 0; i < 7; i++)
				entries[i] = new LocatorEntry(block.Slice(i * 24, 24));
		}
		public LocatorEntry this[int index] => entries[index];
	}
}
