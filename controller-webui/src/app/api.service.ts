import { Injectable } from '@angular/core';
import { BehaviorSubject, Observable, shareReplay } from 'rxjs';

declare const Paho: any;

export interface StateMessage {
  stationId: string;
  state: string;
}

@Injectable({
  providedIn: 'root'
})
export class ApiService {
  private ws!: WebSocket;

  private _messages: BehaviorSubject<StateMessage> = new BehaviorSubject({stationId: 'Total', state: '0'});
  get messages$(): Observable<StateMessage> {
    return this._messages.asObservable().pipe(shareReplay());
  }

  constructor() { }

  async connect() {
    return new Promise((resolve, reject) => {
      console.log('Trying to open a WebSocket connection...');
      // this.ws = new WebSocket(`ws://192.168.4.1/ws`);
      this.ws = new WebSocket(`ws://${location.host}/ws`);
      this.ws.onopen    = (event: Event) => resolve(event);
      this.ws.onclose   = this.onClose.bind(this);
      this.ws.onmessage = this.onMessage.bind(this);
    });
  }

  private onClose(event: Event) {
    console.log('Connection closed');
    setTimeout(this.connect.bind(this), 2000);
  }

  private onMessage(event: MessageEvent) {
    const payload = JSON.parse(event.data);
    console.log('onMessage: ', payload);

    const stationId: string = payload.stationId;
    this._messages.next({ stationId: stationId, state: payload.state });
  }

  reset() {
    this.send('0', '0');
  }

  send(stationId: string, message: string) {
    return this.ws.send(JSON.stringify({ stationId, message }));
  }
}
