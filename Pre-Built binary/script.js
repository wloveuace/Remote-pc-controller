document.addEventListener("DOMContentLoaded", () => {
    const killBtn = document.getElementById("killSwitch");
    const sleepBtn = document.getElementById("sleepSwitch");
    const muteBtn = document.getElementById("muteSwitch");
    const volumeSlider = document.getElementById("range");

    function sendCommand(command, value = null) {
        let url = `/${command}`;
        if (value !== null) url += `?value=${value}`;

        fetch(url, { method: "GET" })
            .then(() => console.log(`Sent: ${command}${value !== null ? " with value " + value : ""}`))
            .catch(err => console.error("Error:", err));
    }

    killBtn.addEventListener("click", () => {
        sendCommand("shutdown");
    });

    sleepBtn.addEventListener("click", () => {
        sendCommand("sleep");
    });

    muteBtn.addEventListener("click", () => {
        sendCommand("mute");
    });

    // Send volume on slider change
    volumeSlider.addEventListener("input", () => {
        const volume = volumeSlider.value;
        sendCommand("volume", volume);
    });
});