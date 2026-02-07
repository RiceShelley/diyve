
module clip(overhang, clip_width, clip_height, base_height, base_thickness) {
    linear_extrude(clip_width) {
        polygon([[0, 0], [0, overhang], [clip_height, 0]]);
    }
    translate([-(base_height-clip_height), -overhang, 0])
        cube([base_height, base_thickness, clip_width]);
}

clip(
    overhang = 1, 
    clip_width = 5,
    clip_height = 3,
    base_height = 10,
    base_thickness = 1
);