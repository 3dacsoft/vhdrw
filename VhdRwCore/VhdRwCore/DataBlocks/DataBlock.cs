using System;
using System.Text;
using System.Runtime.CompilerServices;

namespace VhdRwCore
{
	public class DataBlock
	{
		protected readonly byte[] data;

		protected DataBlock(byte[] data)
		{
			this.data = data;
		}

		protected string ExtractString(int offset, Encoding encoding, int length)
		{
			if (data[offset] == 0) return string.Empty;
			else return encoding.GetString(data.AsSpan(offset, length)).Trim();
		}
		protected T ExtractNumber<T>(int offset) =>
			EndianCorrect<T>(data.AsSpan(offset, Unsafe.SizeOf<T>()));

		protected Version ExtractVersion(int offset) => new Version(
			EndianCorrect<short>(data.AsSpan(offset, 2)),
			EndianCorrect<short>(data.AsSpan(offset + 2, 2)));

		protected Guid ExtractGuid(int offset) => new Guid(data.AsSpan(offset, 16));

		protected DateTime ExtractTimeStamp(int offset) => 
			(Constants.TimeStampBase + TimeSpan.FromSeconds(ExtractNumber<int>(offset))).ToLocalTime();

		private T EndianCorrect<T>(ReadOnlySpan<byte> bytes)
		{
			if (bytes.Length != Unsafe.SizeOf<T>())
				throw new ArgumentException("Incorrect span size for conversion");

			var byteArray = bytes.ToArray();
			if (BitConverter.IsLittleEndian)
				Array.Reverse(byteArray);

			return Unsafe.As<byte, T>(ref byteArray[0]);
		}

		public static T ExtractNumber<T>(byte[] data, int offset) => new DataBlock(data).ExtractNumber<T>(offset);
	}
}
