import numpy as np, pdb

# P_k|k-1 = F P_k-1|k-1 F_T + Q_k
# 
# x_k is the state of system at time k
# c_k is the control input at time k, in this case the sensormeasurement
# w_k ~ N(0, Q_k)
# x_k = [angle, angle_bias] = F x_k-1 + B c + w_k

class Filter:
    def __init__(self):
        self.current_step: int = 0
        self.Q_p = np.array([
            [0.001, 0.0],
            [0.0, 0.003]
        ])
        self.H = np.array([ # Linear transformation from hidden space to observed space
            1.0,
            0.0
        ])

        self.prev_state = np.array([
            0.0,
            0.0
        ])

        self.prev_cov = np.array([
            [0, 0],
            [0, 0]
        ])

        self.noise = 0.4

        self.steps = 0

    def predict(self, time: float, prev_state: np.array, prev_cov: np.array,  control_input: float):
        F = np.array([
            [ 1, -time ],
            [ 0,    1 ]
        ])
        B = np.array([
            time,
            0
        ])
        Q_k = self.Q_p * time
        #w_k = np.random.multivariate_normal(0, cov=Q_k, shape=(2,))

        current_state = F.dot(prev_state) + B.dot([
            control_input,
            0.0
        ])

        current_cov = F.dot(prev_cov).dot(F.T) + Q_k

        #if np.isnan(current_cov).any() or np.isnan(current_state).any():
        #    pdb.set_trace()


        return current_state, current_cov
    
    def update(self, time: float, control_input: float, angle: float):
        current_state, current_cov = self.predict(time, self.prev_state, self.prev_cov, control_input)
        
        y = angle - self.H.dot(current_state)

        S_k = self.H.dot(current_cov).dot(self.H.T) + self.noise
        
        S_k_inv = 1 / (S_k + 0.000001)

        K = current_cov.dot(self.H.T) * S_k_inv
        
        posterior_state = current_state + (K * y)
        
            
        posterior_cov = (np.array([
            [1 - K[0], 0],
            [-K[1], 1]
        ])).dot(current_cov)

        self.prev_state = posterior_state
        self.prev_cov = posterior_cov
        self.steps += 1

        return posterior_state[0] - posterior_state[1]
        #if self.steps > 10:
        #    pdb.set_trace()


f = Filter()

measurement_noise = 0.01

true_angle = 0.0
measured = 0.0

for step in range(40000):
    if np.random.rand() < 0.5:
        turn = np.random.rand() * 2
    else:
        turn = -np.random.rand()
    true_angle += turn
    measured += turn + np.random.normal(measurement_noise)
    observed_turn = turn + np.random.normal(measurement_noise)
    f.update(1, observed_turn, measured)
    
print('True', true_angle)
print('Measured', measured)
print('Predicted', list(f.prev_state))
