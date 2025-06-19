#ifndef WEBPAGE_H
#define WEBPAGE_H

const char webpage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>

<head>
    <title>Tic Tac Toe</title>
    <style>
        body {
            padding: 0;
            margin: 0;
            font-family: Verdana, Geneva, Tahoma, sans-serif;
            background-color: oklch(12.9% 0.042 264.695);
            color: oklch(96.8% 0.007 247.896);
            touch-action: none;
        }

        #wrapper {
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
            width: 100svw;
            height: 100svh;
        }

        h2 {
            font-size: 8rem;
            text-align: center;
        }

        table {
            border: 0.5rem solid oklch(96.8% 0.007 247.896);
            border-radius: 2rem;
            border-collapse: separate;
            border-spacing: 0;
            overflow: hidden;
        }

        td {
            width: 10rem;
            height: 10rem;
            text-align: center;
            border: 0.2rem solid oklch(96.8% 0.007 247.896);
            font-size: 5rem;
        }

        p {
            font-size: 2.5rem;
        }
    </style>
</head>

<body>
    <div id="wrapper">
        <h2>Tic Tac Toe</h2>
        <table id="board">
            <tr>
                <td></td>
                <td></td>
                <td></td>
            </tr>
            <tr>
                <td></td>
                <td></td>
                <td></td>
            </tr>
            <tr>
                <td></td>
                <td></td>
                <td></td>
            </tr>
        </table>
        <p id="statusText"></p>

    </div>

    <script>
        function setPlayer(field) {
            fetch('/setplayer', {
                method: 'POST',
                body: JSON.stringify(field)
            })
        }

        function fetchGameState() {
            fetch('/gamestate')
                .then(response => response.json())
                .then(data => {
                    const board = data.game;
                    const status = data.status;
                    let table = '';
                    for (let i = 0; i < 3; i++) {
                        table += '<tr>';
                        for (let j = 0; j < 3; j++) {
                            let mark = board[i][j] === 1 ? 'X' : (board[i][j] === 0 ? 'O' : '');
                            table += `<td onclick="setPlayer(${i},${j})">${mark}</td>`;
                        }
                        table += '</tr>';
                    }
                    document.getElementById('board').innerHTML = table;

                    const messages = ['Spieler X ist an der Reihe.', 'Spieler Y ist an der Reihe.', 'X Hat Gewonnen!', 'O Hat Gewonnen!'];
                    document.getElementById('statusText').innerText = messages[status];
                });
        }

        setInterval(fetchGameState, 500);
        fetchGameState();
    </script>
</body>

</html>
)rawliteral";

#endif
