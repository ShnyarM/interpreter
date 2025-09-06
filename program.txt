function recursive(){
    number i = 55;
    print(i);

    print(k);
    k=k+1;
    if(k < 100){
        recursive();
    }
}

number k=0;
recursive();
print(k+100);