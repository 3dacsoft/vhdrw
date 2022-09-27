using System;
using System.IO;
using static VhdRwCore.Constants;

namespace VhdRwCore
{
	class DynamicVhd : VirtualHardDisk
	{
		private readonly DynamicVhdHeader header;

		internal DynamicVhd(FileInfo vhdfile, VhdFooter footer) : base(vhdfile, footer)
		{
			byte[] buffer = new byte[1024];
			vhdstream.Seek(footer.DataOffset, SeekOrigin.Begin);
			vhdstream.Read(buffer, 0, 1024);

			header = new DynamicVhdHeader(buffer);
		}

		public override byte[] ReadSector(long lba)
		{
			var sectorLoc = FindSectorLocation(lba);
			var buffer = new byte[512];

			vhdstream.Seek(sectorLoc, SeekOrigin.Begin);
			vhdstream.Read(buffer);
			return buffer;
		}

		public override void WriteSector(long address, ReadOnlySpan<byte> data)
		{
			if (data.Length > SectorSize) throw new InvalidDataException("Sector data must be no more than 512 bytes");
			var location = FindSectorLocation(address);
			if (location < 0) throw new NotImplementedException("Not creating new blocks yet");

			vhdstream.Seek(location, SeekOrigin.Begin);

			if (data.Length < SectorSize)
			{
				var buffer = new byte[SectorSize];
				data.CopyTo(buffer);
				vhdstream.Write(buffer);
			}
			else vhdstream.Write(data);
		}

		private long FindSectorLocation(long lba)
		{
			if (lba * SectorSize > footer.CurrentSize) throw new IndexOutOfRangeException();

			var sectorsPerBlock = header.BlockSize / SectorSize;
			var blockIndex = lba / sectorsPerBlock;
			var buffer = new byte[SectorSize];

			//Read the value in the BAT entry
			vhdstream.Seek(header.TableOffset + (blockIndex * BATEntrySize), SeekOrigin.Begin);
			vhdstream.Read(buffer, 0, 4);
			var batEntry = DataBlock.ExtractNumber<int>(buffer, 0);
			if (batEntry == EmptyBATEntry) return -1;

			//Pad bitmap to sector boundary if necessary
			var bitmapSize = sectorsPerBlock / 8;
			var offBoundary = bitmapSize % SectorSize;
			if (offBoundary > 0)
				bitmapSize += SectorSize - offBoundary;

			//Calculate absolute byte offset of data block
			var sectorInBlock = lba % sectorsPerBlock;
			return (batEntry * SectorSize) + bitmapSize + (sectorInBlock * SectorSize);
		}
	}
}
