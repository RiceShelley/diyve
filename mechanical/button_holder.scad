button_holder(
    button_width = 7,
    slot_depth = 6,
    slot_width = 2,
    slot_wall_width = 1,
    height = 15,
    base_thickness = 1,
    base_width = 10
);

module button_holder(
    button_width,
    slot_depth,
    slot_width,
    slot_wall_width,
    height,
    base_thickness,
    base_width
) {
    slot_size = button_width + slot_depth * 2;

    slot_sup_len = base_width - (slot_width + 2);

    translate([-slot_sup_len, 0, 0])
        cube([base_width, slot_size, base_thickness]);

    translate([0, 0, base_thickness]) {
        tslot(
            slot_width = slot_width,
            slot_depth = slot_depth,
            slot_height = height,
            slot_wall_width = slot_wall_width,
            support_base_length = slot_sup_len
        );

        translate([0, 20, 0]) {
            mirror([0, 1, 0]) {
                tslot(
                    slot_width = slot_width,
                    slot_depth = slot_depth,
                    slot_height = height,
                    slot_wall_width = slot_wall_width,
                    support_base_length = slot_sup_len
                );
            }
        }
    }
}

module tslot(
    slot_width,
    slot_depth,
    slot_height,
    slot_wall_width,
    support_base_length,
    use_sup = true
) {

    support_width = 2;

    difference() {
        cube([slot_width + slot_wall_width * 2, slot_depth + slot_wall_width, slot_height]);

        translate([slot_wall_width, slot_wall_width, -.5])
            cube([slot_width, slot_depth + 1, slot_height + 1]);
    }

    if (use_sup) {
        translate([0, support_width/2 + (slot_depth + slot_wall_width)/2, 0]) {
            rotate([0, -90, 90]) {
                linear_extrude(support_width) {
                    polygon([[0, 0], [0, support_base_length], [slot_height, 0]]);
                }
            }
        }
    }
}