import { Injectable } from '@angular/core';
import { BehaviorSubject, Observable, shareReplay } from 'rxjs';

declare const Paho: any;

export interface MqttMessage {
  topic: string;
  value: string;
}

@Injectable({
  providedIn: 'root'
})
export class MqttService {
  private ws!: WebSocket;

  private _messages: BehaviorSubject<MqttMessage> = new BehaviorSubject({topic: 'Total', value: '0'});
  get messages$(): Observable<MqttMessage> {
    return this._messages.asObservable().pipe(shareReplay());
  }

  constructor() { }

  async connect() {
    return new Promise((resolve, reject) => {
      console.log('Trying to open a WebSocket connection...');
      this.ws = new WebSocket(`ws://192.168.4.1/ws`);
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

    const topic: string = payload.topic.replace('state/', '');
    this._messages.next({ topic: topic, value: payload.message });
  }

  reset() {
    this.send('ctrl', '0');
  }

  endGame() {
    this.send('ctrl', '1');
  }

  send(topic: string, message: string) {
    return this.ws.send(JSON.stringify({ topic, message }));
  }
}
