import profile
from cv2 import reduce
from sympy import true
import taichi as ti
from celestial_objects import Star, Planet, Comet

if __name__ == "__main__":

    ti.init(arch=ti.gpu, kernel_profiler = False)

    # control
    paused = False
    export_images = False

    # stars and planets
    stars = Star(N=2, mass=1000)
    
    stars.initialize(0.5, 0.5, 0.2, 10)
    planets = Planet(N=20000, mass=1)
    planets.initialize(0.5, 0.5, 0.4, 10)
    
    comets = Comet(N=10, mass=100)
    comets.initialize(0, 0, 0, 20)

    # GUI
    my_gui = ti.GUI("Galaxy", (800, 800))
    h = 5e-5 # time-step size
    i = 0
    
    add_weight_button_event = my_gui.button('Add Weight')
    reduce_weight_button_event = my_gui.button('Reduce Weight')
    
    # ti.profiler.clear_kernel_profiler_info()
    while my_gui.running:
    # for time in range(100):
        for e in my_gui.get_events(ti.GUI.PRESS):
            if e.key == ti.GUI.ESCAPE:
                exit()
            elif e.key == ti.GUI.SPACE:
                paused = not paused
                print("paused =", paused)
            elif e.key == 'r':
                stars.initialize(0.5, 0.5, 0.2, 10)
                planets.initialize(0.5, 0.5, 0.4, 10)
                comets.initialize(0, 0, 0, 20)
                i = 0
            elif e.key == 'i':
                export_images = not export_images
            elif e.key == ti.GUI.RMB:
                stars.add_one_star(my_gui.get_cursor_pos())
                print('INFO: Add one star to the galaxy')
            elif e.key == add_weight_button_event:
                stars.adjust_weight(100)   
            elif e.key == reduce_weight_button_event:
                stars.adjust_weight(-100)
            
        if not paused:
            stars.computeForce()
            planets.computeForce(stars, comets)
            comets.computeForce()
            for celestial_obj in (stars, planets, comets):
                celestial_obj.update(h)
            i += 1
            # if i % 100 == 0:
            #     stars.debug_log()

        stars.display(my_gui, radius=10, color=0xffd500)
        planets.display(my_gui)
        comets.display(my_gui, radius=4, color=0x0000ff)
        
        if export_images:
            my_gui.show(f"images\output_{i:05}.png")
        else:
            my_gui.show()
            
    # ti.profiler.print_kernel_profiler_info('count')