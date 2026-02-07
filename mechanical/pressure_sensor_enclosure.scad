// --- Parameters (Adjust these to fit your needs) ---
size = 25;           // Total width/length of the box
height = 10;         // Total height of the box
wall = 1;            // Wall thickness
hole_w = 10;         // Width of the rectangular cutout
hole_h = 5;         // Height of the rectangular cutout
lid_thick = 3;       // Thickness of the flat plate/lid

$fn = 64;            // Smoothness for any rounded edges (if added)

// --- Rendering Controls ---
render_box = false;
render_lid = true;

color([0, 1, 0, 0.2]) {
// 1. The Main Box
if (render_box) {
    difference() {
        // Outer shell
        cube([size, size, height]);

        // Inner cavity (subtracting a smaller cube from the top)
        translate([wall, wall, wall])
            cube([size - 2*wall, size - 2*wall, height]);

        // Rectangular hole in the bottom face
        // Positioned roughly like the image (offset to one side)
        translate([(size - hole_w)/2, wall + 6, -1])
            cube([hole_w, hole_h, wall + 2]);
    }
}
}

lid_tolerance = .5;
lid_size = size + 2*wall + 2*lid_tolerance;
lid_height = 3;

color([1, 0, 0]) {
    translate([-(wall+lid_tolerance), -(wall+lid_tolerance), -lid_height+5]) {

        if (render_lid) {
            union() {
                crosshatch(
                    size=[lid_size, lid_size, 1],
                    outer_wall_width = 2,
                    line_thickness = 1,
                    line_spacing = 4
                );
                difference() {
                    cube([lid_size, lid_size, lid_height]);

                    translate([wall, wall, -lid_height])
                        cube([lid_size - 2*wall, lid_size - 2*wall, lid_height*2.5]);
                }
            }
        }
    }
}


// Create the crosshatch pattern as a cutout
module crosshatch(size, outer_wall_width, line_thickness, line_spacing) {
    hatch_angle1 = 45;
    hatch_angle2 = -45;
    hyp = sqrt(size[0]^2 + size[1]^2);
    union() {
        difference() {
            cube(size);
            translate([outer_wall_width, outer_wall_width, -size[2]/2])
                cube([size[0] - 2*outer_wall_width, size[1] - 2*outer_wall_width, size[2]*2]);
        }
        intersection() {
            cube(size);
            union() {
                // First set of parallel lines
                for (i = [0:line_spacing:size[1]*size[0]*2]) {
                    translate([i, 0, 0])
                    rotate([0, 0, hatch_angle1])
                    cube([line_thickness, hyp, size[2]]);
                }
                // Second set of parallel lines
                for (i = [-size[0]*size[1]:line_spacing:size[0]*size[1]]) {
                    translate([i, 0, 0])
                    rotate([0, 0, hatch_angle2])
                    cube([line_thickness, hyp, size[2]]); // Thin cube to act as a line
                }
            }
        }
    }
}