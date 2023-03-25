class RL {
    constructor (onnxFile, numObservations, numPreviousObservations) {
        this.numObservations = numObservations
        this.numPreviousObservations = numPreviousObservations
        
        this.totalState = []
        for(let i = 0; i < numPreviousObservations; i++) {
            this.totalState.push(Array(numObservations).fill(0))
        }
        // TODO: Add path to model in args
        // TODO: Load model below
        
        ort.InferenceSession.create(`./models/${onnxFile}`).then((session) => {
            this.ortSession = session
          })
    }

    async act (currentState) {
        // state is a list of floats: num_prev_states x num_sensors
        this.totalState = this.totalState.slice(1, this.totalState.length).concat([currentState])
        const stateTensor = new ort.Tensor('float32', Float32Array.from(this.totalState.flat()), [1, this.numPreviousObservations * this.numObservations])
        const {logprobs} = await this.ortSession.run({
            state: stateTensor
        })
        const maxProb = Math.max(...(logprobs.data))
        
        return logprobs.data.indexOf(maxProb)
        // TODO: do forward pass with this.ortSession to get action..
        // return action
    }
}

window.rl = new RL('test2.onnx', 4, 10)

function nextRl () {
    let dists = Array.from(Alpine.store('dists'))
    console.log(dists)
    dists = [
        (dists[0] < 200) * 1.0, //| dists[1]
        (dists[2] < 150) * 1.0,
        (dists[3] < 150) * 1.0, 
    ]

    const action2timeout = {
        0: 50,
        1: 10,
        2: 10,
        3: 300,
        4: 300
    }

    const state = dists.concat([0])
    console.log(state)
    rl.act(state).then((action) => {
        if(Alpine.store('stopAll')) {
            writeAction(0)
        } else {
            writeAction(action)
        
            setTimeout(nextRl, action2timeout[action])
        }
        
    })
}