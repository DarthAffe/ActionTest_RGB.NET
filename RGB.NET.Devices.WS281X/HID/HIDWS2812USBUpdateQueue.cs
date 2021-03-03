using System;
using System.Collections.Generic;
using System.Linq;
using HidSharp;
using RGB.NET.Core;

namespace RGB.NET.Devices.WS281X.HID
{
    // ReSharper disable once InconsistentNaming
    /// <inheritdoc cref="UpdateQueue" />
    /// <summary>
    /// Represents the update-queue performing updates for HID WS2812 devices.
    /// </summary>
    public class HIDWS2812USBUpdateQueue : UpdateQueue
    {
        #region Constants

        private const int OFFSET_MULTIPLIER = 60;
        private byte COUNT_COMMAND = 0x01;
        private const byte SET_COMMAND = 0x02;

        #endregion

        #region Properties & Fields

        private readonly byte[] _sendBuffer;
        private readonly Dictionary<int, byte[]> _dataBuffer = new();
        private readonly HidDevice _device;
        private HidStream? _stream;

        #endregion

        #region Constructors

        public HIDWS2812USBUpdateQueue(IDeviceUpdateTrigger updateTrigger, HidDevice device)
            : base(updateTrigger)
        {
            this._device = device;

            _sendBuffer = new byte[_device.GetMaxInputReportLength() - 1];
        }

        #endregion

        #region Methods

        protected override void Update(Dictionary<object, Color> dataSet)
        {
            foreach (IGrouping<int, ((int _, int index), Color color)> channelData in dataSet.Select(x => (((int channel, int index))x.Key, x.Value)).GroupBy(x => x.Item1.channel))
            {
                int channel = channelData.Key;
                if (!_dataBuffer.TryGetValue(channel, out byte[]? dataBuffer)) continue;
                Span<byte> buffer = dataBuffer;

                foreach (((int _, int index), Color color) in channelData)
                {
                    (byte _, byte r, byte g, byte b) = color.GetRGBBytes();
                    int offset = index * 3;
                    buffer[offset] = r;
                    buffer[offset + 1] = g;
                    buffer[offset + 2] = b;
                }

                Span<byte> sendBuffer = _sendBuffer;
                int packages = dataBuffer.Length / OFFSET_MULTIPLIER;
                if ((packages * OFFSET_MULTIPLIER) < buffer.Length) packages++;
                for (int i = 0; i < packages; i++)
                {
                    int offset = i * OFFSET_MULTIPLIER;
                    int length = Math.Min(buffer.Length - offset, OFFSET_MULTIPLIER);
                    sendBuffer[0] = 0x00;
                    sendBuffer[1] = (byte)((channel << 4) | SET_COMMAND);
                    sendBuffer[2] = i == (packages - 1) ? 1 : 0;
                    sendBuffer[3] = (byte)i;
                    buffer.Slice(offset, length).CopyTo(sendBuffer.Slice(4, length));
                    Send(_sendBuffer);
                }
            }
        }

        private void Send(byte[] command) => _stream!.Write(command);
        private byte[] Read() => _stream!.Read();

        internal IEnumerable<(int channel, int ledCount)> GetChannels()
        {
            _stream ??= _device.Open();

            Send(new byte[2] { 0x00, COUNT_COMMAND });
            int channelCount = Read()[1];

            for (int i = 1; i <= channelCount; i++)
            {
                byte[] channelLedCountCommand = { 0x00, (byte)((i << 4) | COUNT_COMMAND) };
                Send(channelLedCountCommand);
                int ledCount = Read()[1];
                _dataBuffer[i] = new byte[ledCount * 3];
                if (ledCount > 0)
                    yield return (i, ledCount);
            }
        }

        public override void Dispose()
        {
            base.Dispose();

            _stream?.Dispose();
        }

        #endregion
    }
}
