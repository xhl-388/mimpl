import imp
from re import M
from turtle import shape
import taichi as ti

# constants
G = 1
PI = 3.1415926
max_star_num = 10

@ti.data_oriented
class CelestialObject:
    def __init__(self, N, mass) -> None:
        self.n = ti.field(ti.i32,shape=())
        self.n[None] = N
        self.m = ti.field(ti.f32, shape=())
        self.m[None] = mass
        
        self.pos = ti.Vector.field(2, ti.f32, shape=N)
        self.vel = ti.Vector.field(2, ti.f32, shape=N)
        self.force = ti.Vector.field(2, ti.f32, shape=N)
        

    def display(self, gui, radius=2, color=0xffffff):
        gui.circles(self.pos.to_numpy(), radius=radius, color=color)

    @ti.func
    def Pos(self):
        return self.pos

    @ti.func
    def Mass(self):
        return self.m[None]

    @ti.func
    def Number(self):
        return self.n[None]

    @ti.func
    def clearForce(self):
        for i in range(self.n[None]):
            self.force[i] = ti.Vector([0.0, 0.0])

    @ti.kernel
    def initialize(self, center_x: ti.f32, center_y: ti.f32, size: ti.f32, init_speed: ti.f32):
        for i in range(self.n[None]):
            if self.n[None] == 1:
                self.pos[i] = ti.Vector([center_x, center_y])
                self.vel[i] = ti.Vector([0.0, 0.0])
            else:
                theta, r = self.generateThetaAndR(i, self.n[None])
                offset_dir = ti.Vector([ti.cos(theta), ti.sin(theta)])
                center = ti.Vector([center_x, center_y])
                self.pos[i] = center + r * offset_dir * size
                self.vel[i] = ti.Vector([-offset_dir[1], offset_dir[0]]) * init_speed

    @ti.kernel
    def computeForce(self):
        self.clearForce()
        for i in range(self.n[None]):
            p = self.pos[i]
            for j in range(self.n[None]):
                if j != i:
                    diff = self.pos[j] - p
                    r = diff.norm(1e-2)
                    self.force[i] += G * self.Mass() * self.Mass() * diff / r**3

    @ti.kernel
    def update(self, h: ti.f32):
        for i in range(self.n[None]):
            self.vel[i] += h * self.force[i] / self.Mass()
            self.pos[i] += h * self.vel[i]
            
    def debug_log(self):
        print("pos:",self.pos,"vel:",self.vel,"force:",self.force)

@ti.data_oriented
class Star(CelestialObject):
    def __init__(self, N, mass) -> None:
        self.n = ti.field(ti.i32,shape=())
        self.n[None] = N
        self.m = ti.field(ti.i32,shape=())
        self.m[None] = mass
        
        self.pos = ti.Vector.field(2, ti.f32)
        self.vel = ti.Vector.field(2, ti.f32)
        self.force = ti.Vector.field(2, ti.f32)
        
        ti.root.dynamic(ti.j, max_star_num, 32).place(self.pos)
        ti.root.dynamic(ti.j, max_star_num, 32).place(self.vel)
        ti.root.dynamic(ti.j, max_star_num, 32).place(self.force)

    @staticmethod
    @ti.func
    def generateThetaAndR(i, n):
        theta = 2*PI*i/ti.cast(n, ti.f32)
        r = 1  
        return theta, r   
    
    @ti.kernel
    def copy(self, src:ti.template(), dst:ti.template()):
        for i in ti.grouped(src):
            dst[i]=src[i]
    
    def add_one_star(self, pos):   
        if self.n[None] < max_star_num:
            self.add_one_capacity(pos[0], pos[1])
        
    def adjust_weight(self, val):
        self.m[None] += val
        if self.m[None] < 100:
            self.m[None] = 100
        print(f'INFO: Stars\' Weight now: {self.m[None]}')
        
    
    @ti.kernel
    def add_one_capacity(self, posx:float, posy:float):
        self.pos[self.n[None]] = ti.Vector([posx, posy])
        self.vel[self.n[None]] = ti.Vector([0.0, 0.0])
        self.force[self.n[None]] = ti.Vector([0.0, 0.0])
        self.n[None] += 1
    
    def display(self, gui, radius=2, color=0xffffff):
        gui.circles(self.pos.to_numpy()[0:self.n[None]], radius=radius, color=color)
    

@ti.data_oriented
class Planet(CelestialObject):
    def __init__(self, N, mass) -> None:
        super().__init__(N, mass)
        pass

    @staticmethod
    @ti.func
    def generateThetaAndR(i,n):
        theta = 2 * PI * ti.random()  # theta \in (0, 2PI)
        r = (ti.sqrt(ti.random()) * 0.4 + 0.6)  # r \in (0.6,1)    
        return theta, r   

    @ti.kernel
    def computeForce(self, stars: ti.template(), comets: ti.template()):
        self.clearForce()
        for i in range(self.n[None]):
            p = self.pos[i]

            for j in range(self.n[None]):
                if i != j:
                    diff = self.pos[j] - p
                    r = diff.norm(1e-2)
                    self.force[i] += G * self.Mass() * self.Mass() * diff / r**3

            for j in range(stars.Number()):
                diff = stars.Pos()[j] - p
                r = diff.norm(1e-2)
                self.force[i] += G * self.Mass() * stars.Mass() * diff / r**3
            
            for j in range(comets.Number()):
                diff = comets.Pos()[j] - p
                r = diff.norm(1e-2)
                self.force[i] += G * self.Mass() * comets.Mass() * diff / r**3
                
@ti.data_oriented
class Comet(CelestialObject):
    def __init__(self, N, mass) -> None:
        super().__init__(N, mass)
        pass
    
    def computeForce(self):
        pass
    
    @ti.kernel
    def initialize(self, center_x: ti.f32, center_y: ti.f32, size: ti.f32, init_speed: ti.f32):
        x = 0
        for i in range(self.n[None]):
            y = i / self.n[None]
            self.pos[i] = [x,y]
            self.vel[i] = init_speed * ti.Vector([1,0])