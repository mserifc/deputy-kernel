const c = document.querySelector('canvas');
const ctx = c.getContext('2d');

c.width = screen.width;
c.height = screen.height;

var key = {
	up: 'w',
	down: 's',
	left: 'a',
	right: 'd',
	state: {
		up: false,
		down: false,
		left: false,
		right: false
	}
};

class Sprite {
	constructor() {
		this.image = ' ';
		this.x = 0;
		this.y = 0;
		this.width = 0;
		this.height = 0;
	}
	setSize(_width, _height) {
		this.width = _width;
		this.height = _height;
	}
	setPosition(_x, _y) {
		this.x = _x;
		this.y = _y;
	}
	load() {
		ctx.drawImage(document.getElementById(this.image), this.x, this.y, this.width, this.height);
		requestAnimationFrame(() => { this.load(); });
	}
}

ctx.fillStyle = 'darkblue';
ctx.fillRect(0, 0, c.width, c.height);

function addAsset(_type, _id, _src) {
	if (_type == 'image') {
		document.getElementById('assets').innerHTML += `<img id="${_id}" src="${_src}">`;
	} else if (_type == 'sound') {
		document.getElementById('assets').innerHTML += `<audio id="${_id}"><source src="${_src}"></audio>`;
	} else {
		console.error('Error: Asset type undefined.');
	}
}

function getAsset(_id) {
	return document.getElementById(_id);
}

document.addEventListener('keydown', (e) => {
	switch (e.key) {
		case key.up:
			key.state.up = true;
			break;
		case key.down:
			key.state.down = true;
			break;
		case key.left:
			key.state.left = true;
			break;
		case key.right:
			key.state.right = true;
			break;
	}
});

document.addEventListener('keyup', (e) => {
	switch (e.key) {
		case key.up:
			key.state.up = false;
			break;
		case key.down:
			key.state.down = false;
			break;
		case key.left:
			key.state.left = false;
			break;
		case key.right:
			key.state.right = false;
			break;
	}
});

addAsset('image', 'spaceshipImage', 'https://media.indiedb.com/images/games/1/68/67090/spaceship.1.png');

const playerSpaceship = new Sprite();
playerSpaceship.image = 'spaceshipImage';
playerSpaceship.setSize(128, 128);
playerSpaceship.load();