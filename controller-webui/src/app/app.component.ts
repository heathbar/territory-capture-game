import { Component } from '@angular/core';
import { ApiService, StateMessage } from './api.service';
import { Observable, combineLatest, distinctUntilKeyChanged, filter, firstValueFrom, interval, map, of, pairwise, scan, shareReplay, startWith, take, takeWhile, tap, withLatestFrom } from 'rxjs';

const RED = 1;
const BLUE = 2;

@Component({
  selector: 'app-root',
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.scss']
})
export class AppComponent {

  scoreToWinModel: any = '600';
  get scoreToWin() {
    return parseInt(this.scoreToWinModel);
  }

  messages$ = this.apiService.messages$.pipe(
    filter(msg => msg.stationId !== '0'),
  );

  state$ = this.messages$.pipe(
    distinctUntilStationChanged(),
    scan((acc: StationState[], message) => {
      const sum = acc.find(station => station.name === 'Total') as StationState;
      let station = acc.find(station => station.name === message.stationId);

      if (station) {
        station.holder = parseInt(message.state);
      } else {
        station = { name: message.stationId, holder: parseInt(message.state), red: 0, blue: 0 };
        acc.push(station);
      }

      acc.sort(this.sortStates);
      return acc;
    }, [this.makeTotalStation()]),
  );

  scores$: Observable<StationState[]> = combineLatest([
    interval(1000).pipe(startWith(0)),
    this.state$
  ]).pipe(
    map(([i, state]) => {
      const sum = state.find(station => station.name === 'Total') as StationState;
   
      state.forEach(station => {
        if (station.holder === RED) {
          station.red++;
          sum.red++;
        } else if (station.holder === BLUE) {
          station.blue++;
          sum.blue++;
        }
      });
      return state;
    }),
    shareReplay()
  );


  gameState$ = this.scores$.pipe(
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

  constructor(private apiService: ApiService) { }

  async ngOnInit() {
    await this.apiService.connect();
    this.reset();
  }

  async reset() {
    const states = await firstValueFrom(this.scores$)

    states.forEach(state => {
      state.holder = 0;
      state.red = 0;
      state.blue = 0;
    });


    this.apiService.reset();
  }

  endGame() {
    this.apiService.reset();
  }

  private makeTotalStation() {
    return { name: 'Total', holder: 0, red: 0, blue: 0 };
  }

  private sortStates(a: StationState, b: StationState) {
    if (a.name === 'Total' || a.name < b.name) {
      return -1;
    } else if (b.name === 'Total' || a.name > b.name) {
      return 1;
    }
    return 0;
  }
}

function distinctUntilStationChanged() {
  const stationHolders: any = {};

  return function<T extends StateMessage>(source: Observable<T>): Observable<T> {
    return new Observable(subscriber => {
      source.subscribe({
        next(station: T) {
          if (stationHolders[station.stationId] === undefined || stationHolders[station.stationId] !== station.state) {
            stationHolders[station.stationId] = station.state;
            subscriber.next(station);
          }
        },
        error(error) {
          subscriber.error(error);
        },
        complete() {
          subscriber.complete();
        }
      })
    });
  }
}


interface StationState {
  name: string;
  holder: number;
  red: number;
  blue: number;
}