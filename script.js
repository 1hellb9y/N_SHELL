const copyButton = document.getElementById("copyButton");

const commands = `git clone https://github.com/YOUR_USERNAME/n-shell.git
cd n-shell
make
./nshell`;

copyButton.addEventListener("click", async () => {

    try {

        await navigator.clipboard.writeText(commands);

        copyButton.textContent = "Copied!";

        setTimeout(() => {
            copyButton.textContent = "Copy";
        }, 1500);

    } catch (error) {

        copyButton.textContent = "Failed";

        setTimeout(() => {
            copyButton.textContent = "Copy";
        }, 1500);

    }

});