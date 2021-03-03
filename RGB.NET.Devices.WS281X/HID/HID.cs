using System;
using System.Collections.Generic;
using System.Linq;
using HidSharp;
using RGB.NET.Core;

namespace RGB.NET.Devices.WS281X.HID
{
    internal static class HID
    {
        #region Constants

        private const int VENDOR_ID = 0xCAFE;
        private const int PRODUCT_ID = 0x4004;

        #endregion

        #region Methods

        internal static IEnumerable<HidDevice> GetDevices()
        {
            return DeviceList.Local.GetHidDevices(vendorID: VENDOR_ID, productID: PRODUCT_ID)
                             .Where(d => d.GetManufacturer().Equals("RGB.NET", StringComparison.OrdinalIgnoreCase));
        }

        internal static IEnumerable<IRGBDevice> CreateDevices(IDeviceUpdateTrigger updateTrigger, HidDevice hidDevice)
        {
            HIDWS2812USBUpdateQueue queue = new(updateTrigger, hidDevice);
            IEnumerable<(int channel, int ledCount)> channels = queue.GetChannels();
            int counter = 0;
            foreach ((int channel, int ledCount) in channels)
            {
                HIDWS2812USBDevice device = new(new HIDWS2812USBDeviceInfo($"RGB.NET WS2812 USB [{++counter}]"), queue, channel);
                device.Initialize(ledCount);
                yield return device;
            }
        }

        #endregion
    }
}
