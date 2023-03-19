class RL {
    constructor (numObservations, numPreviousObservations) {
        this.numPreviousObservations = numPreviousObservations
        this.totalState = []
        for(let i = 0; i < numPreviousObservations; i++) {
            this.totalState.push(Array(numObservations).fill(0))
        }
    }

    act (currentState) {
        // state is a list of floats: num_prev_states x num_sensors
        this.totalState = this.totalState.slice(1, this.totalState.length).concat([currentState])
        // do forward pass to get action..
        // return action
    }
}