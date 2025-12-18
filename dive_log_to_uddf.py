import struct
import xml.etree.cElementTree as ET
import argparse
import sys
from datetime import datetime


class dive_log_entry:
    """Mirrors the C dive_log_entry_t struct provided in the source."""

    def __init__(
        self, year, month, day, hour, minute, second, temperature_c, depth_m
    ):
        # Fields mapped from the C struct timestamp and float fields
        self.year = year
        self.month = month
        self.day = day
        self.hour = hour
        self.minute = minute
        self.second = second
        self.temperature_c = temperature_c
        self.depth_m = depth_m

        # Internal datetime object for easier calculation
        self.dt = datetime(year, month, day, hour, minute, second)

    def __repr__(self):
        return f"DiveLogEntry({self.dt.isoformat()}, Temp: {self.temperature_c}C, Depth: {self.depth_m}m)"


def read_dive_log(file_path):
    """Reads binary file and returns a list of dive_log_entry objects based on C pack logic."""
    # Format: H (uint16), 5B (uint8), 2f (float)
    entry_format = "<HBBBBBff"
    entry_size = struct.calcsize(entry_format)
    entries = []

    try:
        with open(file_path, "rb") as f:
            while chunk := f.read(entry_size):
                if len(chunk) == entry_size:
                    data = struct.unpack(entry_format, chunk)
                    entries.append(dive_log_entry(*data))
    except FileNotFoundError:
        print(f"Error: Input file '{file_path}' not found.")
        sys.exit(1)
    return entries


def write_to_uddf(entries, output_file):
    """Converts entries to UDDF format using the provided XML structure logic."""
    if not entries:
        print("No entries found in the binary file to convert.")
        return

    uddf_doc = ET.Element("uddf", version="3.2.0")

    # Generator Section
    generator = ET.SubElement(uddf_doc, "generator")
    ET.SubElement(generator, "name").text = "BinToUDDF"
    ET.SubElement(generator, "manufacturer").text = "Custom_Logger"
    ET.SubElement(generator, "version").text = "0.1.0"
    ET.SubElement(generator, "datetime").text = datetime.now().strftime(
        "%Y-%m-%dT%H:%M:%S"
    )

    # Gas Definitions (Default to Air)
    gas_defs = ET.SubElement(uddf_doc, "gasdefinitions")
    air_mix = ET.SubElement(gas_defs, "mix", id="air")
    ET.SubElement(air_mix, "o2").text = "0.210"
    ET.SubElement(air_mix, "he").text = "0.000"

    # Profile Data
    profile_data = ET.SubElement(uddf_doc, "profiledata")
    repetition_group = ET.SubElement(profile_data, "repetitiongroup")
    dive = ET.SubElement(repetition_group, "dive")

    # Dive Metadata
    info_before = ET.SubElement(dive, "informationbeforedive")
    ET.SubElement(info_before, "datetime").text = entries[0].dt.strftime(
        "%Y-%m-%dT%H:%M:%S"
    )

    samples = ET.SubElement(dive, "samples")
    start_time = entries[0].dt
    max_depth = 0.0
    min_temp = 1000.0

    # Process individual waypoints
    for entry in entries:
        waypoint = ET.SubElement(samples, "waypoint")
        divetime_sec = (entry.dt - start_time).total_seconds()

        ET.SubElement(waypoint, "divetime").text = f"{divetime_sec:.0f}"
        ET.SubElement(waypoint, "depth").text = f"{entry.depth_m:.2f}"
        ET.SubElement(waypoint, "temperature").text = (
            f"{entry.temperature_c:.2f}"
        )

        if entry.depth_m > max_depth:
            max_depth = entry.depth_m
        if entry.temperature_c < min_temp:
            min_temp = entry.temperature_c

    # Post-dive summary
    info_after = ET.SubElement(dive, "informationafterdive")
    ET.SubElement(info_after, "greatestdepth").text = f"{max_depth:.2f}"
    ET.SubElement(info_after, "lowesttemperature").text = f"{min_temp:.2f}"
    ET.SubElement(info_after, "diveduration").text = (
        f"{(entries[-1].dt - start_time).total_seconds():.0f}"
    )

    # Export to XML
    tree = ET.ElementTree(uddf_doc)
    ET.indent(tree, "  ")
    tree.write(output_file, encoding="utf-8", xml_declaration=False)
    print(f"Successfully converted {len(entries)} entries to {output_file}")


def main():
    parser = argparse.ArgumentParser(
        description="Convert custom binary dive logs to UDDF format."
    )
    parser.add_argument(
        "-i",
        "--input-file",
        required=True,
        help="Path to the .bin dive log file",
    )
    parser.add_argument(
        "-o",
        "--output-file",
        required=True,
        help="Path for the output .uddf file",
    )

    args = parser.parse_args()

    # Process the files
    entries = read_dive_log(args.input_file)
    write_to_uddf(entries, args.output_file)


if __name__ == "__main__":
    main()
