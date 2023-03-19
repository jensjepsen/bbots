function waitFor(condition, next, frequency) {
    setTimeout(
        () => {
            console.log('waiting')
            if (condition()) {
                next()
            } else {
                waitFor(condition, next, frequency)
            }
        }, frequency
    )
}

function modmod (e, d) {
    return e - Math.floor(e/d) * d
}
  

function executePlan(plan, step) {
    step = step || 0
    command = plan[step]
    command.running = true
    console.log(Date.now())
    const nextStep = () => {
        command.running = false
        if(step + 1 < plan.length) {
            executePlan(plan, step + 1)
        }
    }
    if (["rotate"].includes(command.name)) {
        let desiredRotation = modmod(Alpine.store('orientation')[0] + command.args[0], 360)
        //desiredRotation = (desiredRotation == 0 || desiredRotation == 360) ? 359 : desiredRotation
        writeCommand(command.name, desiredRotation)
        waitFor(() => Alpine.store('desiredOrientation'), () => setTimeout(nextStep, 1000), 500)
    } else {
        writeCommand(command.name)
        setTimeout(() => {
            writeCommand('stop')
            nextStep()
        }, command.args[0] * 1000)
    }
}

function parseCommand(command) {
    const [name, ...args] =  command.split(' ')
    return {
        name,
        args: args.map(parseFloat),
        running: false
    }
}

function makePlan(input, token) {
    //const url = 'https://hr-poc.openai.azure.com/openai/deployments/text-davinci-003/completions?api-version=2022-12-01'
    const url = 'https://api.openai.com/v1/completions'

    return fetch(url, {
        method: 'POST',
        headers: {
            'Authorization': `Bearer ${token}`,
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({
            model: 'text-davinci-003',
            stop: ['End'],
            max_tokens: 1000,
            prompt: `
There is a robot. The robot can be controlled by issuing commands.
To move the robot forward for x seconds, output "forward x"
To move the robot backward for x seconds, output "backward x"
To turn the robot counter clockwise x degs, output "rotate -x"
To turn the robot clockwise x degs, output "rotate x"

By default, the robot should take small steps of 1 seconds, unless otherwise specified.

Example:

Command: Move the robot forwards for 10 seconds, then turn 45 degrees  right.
Action: forward 0.5, rotate 45
End

Wrong example:

Command: Move the robot forwards
Action: forward
End

Example: 
Command: Move the robot forwards
Action: forward 1

Example:
Command: Turn around
Action: rotate 180
End

Command: ${input}
Action:`
        })
    }).then((response) => {
        return response.json()
    }).then((json) => {
        return json['choices'][0]['text'].split(',').map((s) => parseCommand(s.trim()))
    })
}

function storeToken(token) {
    localStorage.setItem('token', token)
}

function getToken() {
    return localStorage.getItem('token')
}
