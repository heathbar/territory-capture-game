import { Component } from '@angular/core';
import { MqttService } from './mqtt.service';
import { Observable, distinctUntilKeyChanged, filter, firstValueFrom, interval, map, of, scan, shareReplay, take, takeWhile, tap, withLatestFrom } from 'rxjs';

const RED = 1;
const BLUE = 2;

@Component({
  selector: 'app-root',
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.scss']
})
export class AppComponent {

  scoreToWin = 600;

  messages$ = this.mqttService.messages$.pipe(
    filter(msg => msg.topic !== 'ctrl'),
  );

  states$: Observable<NodeState[]> = interval(1000).pipe(
    withLatestFrom(this.messages$),
    scan((acc: NodeState[], [i, message]) => {
      const sum = acc.find(node => node.name === 'Total') as NodeState;
      let node = acc.find(node => node.name === message.topic);

      if (node) {
        node.holder = parseInt(message.value);
      } else {
        node = { name: message.topic, holder: parseInt(message.value), red: 0, blue: 0 };
        acc.push(node);
      }

      acc.forEach(node => {
        if (node.holder === RED) {
          node.red++;
          sum.red++;
        } else if (node.holder === BLUE) {
          node.blue++;
          sum.blue++;
        }
      });
      acc.sort(this.sortStates);
      return acc;
    }, [this.makeTotalNode()]),
    shareReplay()
  );

  gameState$ = this.states$.pipe(
    map(states => {
      if (states[0].red >= this.scoreToWin)
      {
        return { isEndGame: true, winner: 'Red' };
      } else if (states[0].blue >= this.scoreToWin) {
        return { isEndGame: true, winner: 'Blue' };
      } else {
        return { isEndGame: false };
      }
    }),
    distinctUntilKeyChanged('isEndGame'),
    tap(game => {
      if (game.isEndGame) {
        this.endGame();
      }
    }),

  );

  constructor(private mqttService: MqttService) { }

  async ngOnInit() {
    await this.mqttService.connect();
    this.reset();
  }

  async reset() {
    const states = await firstValueFrom(this.states$)

    states.forEach(state => {
      state.holder = 0;
      state.red = 0;
      state.blue = 0;
    });


    this.mqttService.reset();
  }

  endGame() {
    this.mqttService.endGame();
  }

  private makeTotalNode() {
    return { name: 'Total', holder: 0, red: 0, blue: 0 };
  }

  private sortStates(a: NodeState, b: NodeState) {
    if (a.name === 'Total' || a.name < b.name) {
      return -1;
    } else if (b.name === 'Total' || a.name > b.name) {
      return 1;
    }
    return 0;
  }

}

interface NodeState {
  name: string;
  holder: number;
  red: number;
  blue: number;
}