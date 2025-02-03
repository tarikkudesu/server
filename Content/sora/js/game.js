let canvas;
let ctx;
let flowFeild;
let flowFeildAnimation;
let pressed = false;
let moveSpeed = 0.3;
let playerWidth = 150;

const backgrounds = [];
const colors = [`rgba(255, 255, 255, 0)`, `rgba(0, 119, 255, 0.2)`, `rgba(255, 136, 0, 0.2)`];
let backSelector = 0;

const audio = new Audio('audio/Chiasm.mp3');
const warpPad = new Audio('audio/warp_pad.mp3');

let background1 = new Image();
background1.onerror = function() { alert( background1.src + ' failed to load.' ) ; };
background1.src = `assets/level1.png`;
backgrounds.push(background1);

let background2 = new Image();
background2.onerror = function() { alert( background2.src + ' failed to load.' ) ; };
background2.src = `assets/level2.png`;
backgrounds.push(background2);

let background3 = new Image();
background3.onerror = function() { alert( background3.src + ' failed to load.' ) ; };
background3.src = `assets/level3.png`;
backgrounds.push(background3);

destImg = new Image();
destImg.onerror = function() { alert( destImg.src + ' failed to load.' ) ; };
destImg.src = `assets/dest.png`;

obsImg = new Image();
obsImg.onerror = function() { alert( obsImg.src + ' failed to load.' ) ; };
obsImg.src = `assets/tornado.png`;

const   indicatorConrtainerEl = document.getElementById(`indicator-container`);
const   containerEl = document.getElementById(`container`);
const   indicatorEl = document.getElementById(`indicator`);
const   gameoverEl = document.getElementById(`gameover`);
const   colorizeEl = document.getElementById(`colorize`);
const   loadingEl = document.getElementById(`loading`);
const   resumeEl = document.getElementById(`resume`);
const   pauseEl = document.getElementById(`pause`);
const   infoEl = document.getElementById(`info`);

infoEl.remove();
gameoverEl.remove();

function rand(min, max) { return Math.random() * (max - min) + min; }

window.onload = function() {
    audio.loop = true;
    audio.play();
    canvas = document.getElementById(`canvas`);
    ctx = canvas.getContext('2d');
    canvas.width = window.innerWidth;
    canvas.height = window.innerHeight;
    flowFeild = new FlowFeildEffect(ctx, canvas.width, canvas.height);
    pauseEl.addEventListener('click', function() {
        flowFeild.pause = true;
        pauseEl.remove();
        containerEl.appendChild(infoEl);
    });
    resumeEl.addEventListener('click', function() {
        flowFeild.pause = false;
        infoEl.remove();
        containerEl.appendChild(pauseEl);
    });
    flowFeild.animate(0);
}

window.addEventListener('resize', function() {
    location.reload();
    window.location.href = 'index.html';
});

const mouse = {
    x: 0,
    y: 0,
}

window.addEventListener('mousedown', function(e) {
    if (e.button === 0)
        pressed = true;
});

window.addEventListener('mouseup', function(e) {
    if (e.button === 0)
        pressed = false;
});

window.addEventListener('mousemove', (event) => {
    mouse.x = event.clientX;
    mouse.y = event.clientY;
});

class FlowFeildEffect {
    #ctx;
    #width;
    #height;
    #backSize;
    constructor(ctx, width, height) {
        pauseEl.classList.toggle(`hidden`);
        loadingEl.classList.toggle(`hidden`);
        colorizeEl.style.backgroundColor = `${colors[backSelector]}`;
        indicatorConrtainerEl.classList.toggle(`hidden`);
        this.pause = false;
        this.slowDown = 1;
        this.#ctx = ctx;
        this.#width = width;
        this.#height = height;
        this.newMap = true;
        this.timer = 0;
        this.obstacls = [];
        this.obsNumber = 60;
        this.destP = [0, 0, 0];
		this.backP = [0, 0];
        this.#backSize = 5000;
		this.playerP = [this.#width / 2, this.#height / 2];
        this.playerAbsPos = [this.#width / 2, this.#height / 2];
        this.player = document.getElementById(`player`);
        this.player.classList.toggle(`hidden`);
    }
    #generateObs() {
        if (this.newMap === true) {
            for (let i = 0; i < this.obsNumber; i++) {
                const w = rand(0, Math.min(this.#width, this.#height));
                let coor = [rand(-this.#backSize, this.#backSize - w), rand(-this.#backSize, this.#backSize - w), w]
                this.obstacls.push(coor);
            }
        }
    }
    #generateDest() {
        if (this.newMap === true) {
            this.destP[2] = rand(300, 600)
            this.destP[0] = rand(- this.#backSize, this.#backSize - this.destP[2]);
            this.destP[1] = rand(- this.#backSize, this.#backSize - this.destP[2]);
            this.newMap = false;
        }
    }
    #isOverlapping(element, playerAbsPos) {
        const center1 = [element[0] + element[2] / 2, element[1] + element[2] / 2];
        const center2 = [playerAbsPos[0] + playerWidth / 2, playerAbsPos[1] + playerWidth / 2];
        let distance = Math.sqrt(Math.pow(center2[0] - center1[0], 2) + Math.pow(center2[1] - center1[1], 2));
        return distance < element[2] / 2;
    }
    #destOverlap() {
        if (this.#isOverlapping(this.destP, this.playerAbsPos)) {
            this.slowDown = 0.1;
            this.timer += 1;
            if (this.timer === 40)
                warpPad.play();
            if (this.timer === 100) {
                this.#backSize = 10000;
                this.obsNumber += 50;
                if (backSelector < 2)
                    backSelector++;
                colorizeEl.style.backgroundColor = `${colors[backSelector]}`;
                playerWidth = 150;
                moveSpeed += 0.1;
                this.newMap = true;
            }
        } else
            this.timer = 0;
    }
    #overlap() {
        let overlaps = false;
        this.obstacls.forEach(element => {
            if (this.#isOverlapping(element, this.playerAbsPos)) {
                overlaps = true;
                return ;
            }
        });
        if (overlaps == true) {
            this.slowDown = 0.1;
            playerWidth -= 0.3;
            if (playerWidth < 50) {
                this.pause = true;
                containerEl.appendChild(gameoverEl);
                pauseEl.remove();
            }
        } else
            this.slowDown = 1;
    }
    #rotatePlayer() {
        const deltaX = mouse.x - this.playerP[0];
        const deltaY = mouse.y - this.playerP[1];
        const angle = Math.atan2(deltaY, deltaX) * (180 / Math.PI) + 45;
        player.style.transform = `rotate(${angle}deg)`;
    }
    #returnToCenter() {
        const deltaX = this.#width / 2 - this.playerP[0];
        const deltaY = this.#height / 2 - this.playerP[1];
        this.playerP[0] = this.playerP[0] + deltaX * moveSpeed * this.slowDown * 0.3;
        this.playerP[1] = this.playerP[1] + deltaY * moveSpeed * this.slowDown * 0.3;
        this.player.style.left = `${this.playerP[0]}px`;
        this.player.style.top = `${this.playerP[1]}px`;
        this.player.style.width = `${playerWidth}px`;
        this.player.style.height = `${playerWidth}px`;
    }
    #movePlayer() {
        let x = mouse.x;
        let y = mouse.y;
        if (x < this.#width * 0.1)
            x = this.#width * 0.1;
        else if (x > this.#width - this.#width * 0.1 - playerWidth)
            x = this.#width - this.#width * 0.1 - playerWidth;
        if (y < this.#height * 0.1)
            y = this.#height * 0.1;
        else if (y > this.#height - this.#height * 0.1 - playerWidth)
            y = this.#height - this.#height * 0.1 - playerWidth;
        const deltaX = x - this.playerP[0];
        const deltaY = y - this.playerP[1];
        this.playerP[0] = this.playerP[0] + deltaX * moveSpeed * this.slowDown * 0.5;
        this.playerP[1] = this.playerP[1] + deltaY * moveSpeed * this.slowDown * 0.5;
        this.player.style.left = `${this.playerP[0]}px`;
        this.player.style.top = `${this.playerP[1]}px`;
        this.player.style.width = `${playerWidth}px`;
        this.player.style.height = `${playerWidth}px`;
    }
    #updatePlayer() {
        this.#overlap();
        this.#destOverlap();
        if (this.playerAbsPos[0] < - this.#backSize || 
            this.playerAbsPos[0] > this.#backSize   ||
            this.playerAbsPos[1] < - this.#backSize ||
            this.playerAbsPos[1] > this.#backSize) {
                this.slowDown = 0.1;
                playerWidth -= 0.3;
                if (playerWidth < 50) {
                    this.pause = true;
                    containerEl.appendChild(gameoverEl);
                }
        }
        if (!pressed)
            this.#returnToCenter();
        else
        this.#movePlayer();
    }
    #drawBackground() {
        const deltaX = this.playerP[0] - this.#width / 2;
        const deltaY = this.playerP[1] - this.#height / 2;
        this.backP[0] -= deltaX * moveSpeed * this.slowDown * 0.2;
        this.backP[1] -= deltaY * moveSpeed * this.slowDown * 0.2;
        this.playerAbsPos[0] = -this.backP[0] + this.playerP[0];
        this.playerAbsPos[1] = -this.backP[1] + this.playerP[1];
        this.#ctx.drawImage(backgrounds[backSelector], this.backP[0], this.backP[1], this.#backSize, this.#backSize);
        this.#ctx.drawImage(backgrounds[backSelector], this.backP[0] - this.#backSize, this.backP[1], this.#backSize, this.#backSize);
        this.#ctx.drawImage(backgrounds[backSelector], this.backP[0], this.backP[1] - this.#backSize, this.#backSize, this.#backSize);
        this.#ctx.drawImage(backgrounds[backSelector], this.backP[0] - this.#backSize, this.backP[1] - this.#backSize, this.#backSize, this.#backSize);
    }
    #drawDest() {
        this.#ctx.drawImage(destImg, this.destP[0] + this.backP[0], this.destP[1] + this.backP[1], this.destP[2], this.destP[2]);
    }
    #drawObs() {
        this.obstacls.forEach(sub => {
            this.#ctx.drawImage(obsImg, sub[0] + this.backP[0], sub[1] + this.backP[1], sub[2], sub[2]);
        });
    }
    #drawIndicator() {
        const dx = this.playerAbsPos[0] - this.destP[0];
        const dy = this.playerAbsPos[1] - this.destP[1];
        let distance = Math.sqrt(dx * dx + dy * dy) / this.#backSize * 100;
        if (distance > 99)
            distance = 99;
        indicatorEl.style.width = `${Math.trunc(100 - distance)}%`;
    }
    animate() {
        if (this.pause === false) {
            this.#ctx.clearRect(0, 0, this.#width, this.#height);
            this.#generateObs();
            this.#generateDest();
            this.#updatePlayer();
            this.#drawBackground();
            this.#drawObs();
            this.#drawDest();
            this.#drawIndicator();
        }
        this.#rotatePlayer();
        flowFeildAnimation = requestAnimationFrame(this.animate.bind(this));
    }
}
