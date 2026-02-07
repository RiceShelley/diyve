use <button_holder.scad>

render_case_perimeter = false;

case_height = 52;
case_width = 72.5;
base_thickness = 1;

top_button_x = 44;
top_button_y = 23;

side_button_x = 24;
side_button_y = 65;

charger_pcb_holder_x = 45;
charger_pcb_holder_y = 41;

sd_pcb_holder_x = 5;
sd_pcb_holder_y = 44;

rtc_pcb_holder_x = 5;
rtc_pcb_holder_y = 4;


button_holder_height = 10;
button_width = 8;
button_slot_depth = 6;
button_slot_width = 1.5;

// Main bottom plate
base_plate(
    size = [case_height, case_width, base_thickness],
    outer_wall_width = 2,
    line_thickness = 1,
    line_spacing = 10,
    render_case_perimeter = render_case_perimeter
);

button_holder_width = button_width + button_slot_depth * 2;

// Top button holder
translate([top_button_x, top_button_y - (button_holder_width / 2), 0]) {
    button_holder(
        button_width = button_width,
        slot_depth = button_slot_depth,
        slot_width = button_slot_width,
        slot_wall_width = 1,
        height = button_holder_height,
        base_thickness = base_thickness,
        base_width = 10
    );
}

// Side button holder
translate([side_button_x + (button_holder_width / 2), side_button_y, 0]) {
    rotate([0, 0, 90]) {
        button_holder(
            button_width = button_width,
            slot_depth = button_slot_depth,
            slot_width = button_slot_width,
            slot_wall_width = 1,
            height = button_holder_height,
            base_thickness = base_thickness,
            base_width = 10
        );
    }
}

// Charger holder
translate([charger_pcb_holder_x, charger_pcb_holder_y, 0]) {
    charger_pcb_width = 18;
    charger_pcb_holder_height = 15;
    difference() {
        pcb_holder(
            pcb_length = charger_pcb_width,
            slot_depth = 3,
            slot_width = 1.5,
            slot_wall_width = 1,
            height = charger_pcb_holder_height,
            base_thickness = base_thickness,
            base_width = 10
        );
        // Cutout for pads
        translate([-1, 1, base_thickness]) 
            cube([6, charger_pcb_width, 6]);

        // Cutout for feet
        translate([1, 1, -3]) 
            cube([1.5, 5, 6]);

        // Cutout for feet
        translate([1, charger_pcb_width-4, -3]) 
            cube([1.5, 5, 6]);

    }

    // Support
    translate([2.75, charger_pcb_width + 2, 0]) {
        rotate([0, -90, 0]) {
            linear_extrude(2) {
                polygon([[0, 0], [0, 4], [charger_pcb_holder_height, 0]]);
            }
        }
    }
    translate([0.75, 0, 0]) {
        rotate([0, -90, 180]) {
            linear_extrude(2) {
                polygon([[0, 0], [0, 4], [charger_pcb_holder_height, 0]]);
            }
        }
    }
}

// SD holder
translate([sd_pcb_holder_x, sd_pcb_holder_y, 0]) {
    sd_pcb_width = 18;
    sd_pcb_holder_height = 15;
    difference() {
        pcb_holder(
            pcb_length = sd_pcb_width,
            slot_depth = 1.5,
            slot_width = 1.5,
            slot_wall_width = 1,
            height = sd_pcb_holder_height,
            base_thickness = base_thickness,
            base_width = 10
        );
        translate([-1, 1, base_thickness]) 
            cube([6, sd_pcb_width+1, 4]);
    }
    // Support
    translate([3.5, sd_pcb_width + 2, 0]) {
        rotate([0, -90, 0]) {
            linear_extrude(2) {
                polygon([[0, 0], [0, 4], [sd_pcb_holder_height, 0]]);
            }
        }
    }
}

// RTC holder
translate([rtc_pcb_holder_x, rtc_pcb_holder_y, 0]) {
    rtc_pcb_width = 38.5;
    difference() {
        pcb_holder(
            pcb_length = rtc_pcb_width,
            slot_depth = 3,
            slot_width = 1.5,
            slot_wall_width = 1,
            height = 15,
            base_thickness = base_thickness,
            base_width = 10
        );

        translate([-1, -1, base_thickness+5]) 
            cube([6, 6, 15]);
    }
}

module pcb_holder (
    pcb_length,
    slot_depth,
    slot_width,
    slot_wall_width,
    height,
    base_thickness,
    base_width
) {
    //slot_size = button_width + slot_depth * 2;

    slot_sup_len = base_width - (slot_width + 2);

    cube([slot_width + 2*slot_wall_width, pcb_length + slot_wall_width*2, base_thickness]);

    translate([0, 0, base_thickness]) {
        tslot(
            slot_width = slot_width,
            slot_depth = slot_depth,
            slot_height = height,
            slot_wall_width = slot_wall_width,
            support_base_length = slot_sup_len,
            use_sup = false
        );

        translate([0, pcb_length + slot_wall_width*2, 0]) {
            mirror([0, 1, 0]) {
                tslot(
                    slot_width = slot_width,
                    slot_depth = slot_depth,
                    slot_height = height,
                    slot_wall_width = slot_wall_width,
                    support_base_length = slot_sup_len,
                    use_sup=false
                );
            }
        }
    }
}



// Create the crosshatch pattern as a cutout
module base_plate(
    size,
    outer_wall_width,
    line_thickness,
    line_spacing,
    render_case_perimeter = false
) {
    main_sup_width = 10;

    // Main supports
    translate([size[0]/2-main_sup_width/2, 0, 0]) {

        // Temp support to keep base from bending
        translate([main_sup_width/2-1, size[1]/4, base_thickness]) 
            cube([2, size[1]/2, 5]);

        difference () {
            cube([main_sup_width, size[1], size[2]]);
            translate([main_sup_width/2-1, -.1, -base_thickness/2])
                cube([2, 2, base_thickness * 2]);
        }
    }
    translate([0, size[1]/2-main_sup_width/2, 0]) {
        cube([size[0], main_sup_width, size[2]]);

        // Temp support to keep base from bending
        translate([size[0]/2*.5, main_sup_width/2-1, base_thickness])
            cube([size[0]/2, 2, 5]);
    }

    // Supports:

    // Top button holder connector
    translate([size[0]/2, 13, 0]) {
        cube([size[0]/3, 20, size[2]]);
    }

    // SD card holder support
    translate([8, size[1]/2, 0]) {
        cube([14, 31, size[2]]);
    }

    // RTC holder support
    translate([8, 4, 0]) {
        cube([15, 10, size[2]]);
    }

    // Power holder support
    translate([30, 30, 0])
        cube([15, 32, size[2]]);

    translate([33.5, 60, 0])
        cube([15, 5, size[2]]);

    // Outer wall
    if (render_case_perimeter) {
        difference() {
            cube(size);
            translate([outer_wall_width, outer_wall_width, -size[2]/2])
                cube([size[0] - 2*outer_wall_width, size[1] - 2*outer_wall_width, size[2]*2]);
        }
    }
/*
    // First set of parallel lines
    for (i = [0:line_spacing:size[0]-line_spacing]) {
        translate([i, 0, 0])
        cube([line_thickness, size[1], size[2]]);
    }
    // Second set of parallel lines
    for (i = [0:line_spacing:size[1]-line_spacing]) {
        translate([0, i, 0])
        cube([size[0], line_thickness, size[2]]); // Thin cube to act as a line
    }
    */
}
