type Animal = ref object of RootObj
  name: string
  age: int
method vocalize(this: Animal): string {.base.} = "..."
method ageHumanYrs(this: Animal): int {.base.} = this.age

type Dog = ref object of Animal
method vocalize(this: Dog): string = "woof"
method ageHumanYrs(this: Dog): int = this.age * 7

type Cat = ref object of Animal
method vocalize(this: Cat): string = "meow"


var animals: seq[Animal] = @[]
animals.add(Dog(name: "Sparky", age: 10))
animals.add(Cat(name: "Mitten", age: 10))

for a in animals:
  echo a.vocalize()
  echo a.ageHumanYrs()

proc sum(a: int, b:int): int =
    var result = a + b
